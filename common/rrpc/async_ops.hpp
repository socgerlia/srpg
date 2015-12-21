#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/utility/enable_if.hpp>

class chain;

namespace detail {

/// @brief Chained handler connects two handlers together that will
///        be called sequentially.
///
/// @note Type erasure is not performed on Handler1 to allow resolving
///       to the correct asio_handler_invoke via ADL.
template<class FirstHandler, class SecondHandler>
class chained_handler{
public:
	template<class T, class U>
	chained_handler(T&& first, U&& second)
		: _first(std::forward<T>(first))
		, _second(std::forward<U>(second))
	{}

	void operator()(){
		_first();
		_second();
	}

	// template <typename Arg1>
	// void operator()(const Arg1& a1){
	// 	handler1_(a1);
	// 	handler2_();
	// }

	// template <typename Arg1, typename Arg2>
	// void operator()(const Arg1& a1, const Arg2& a2){
	// 	handler1_(a1, a2);
	// 	handler2_();
	// }

// private:
	FirstHandler _first;
	SecondHandler _second;
};

template<class T, class U>
auto make_chained_handler(T&& first, U&& second)
	-> chained_handler<typename std::decay<T>::type, typename std::decay<U>::type>
{
	return chained_handler<typename std::decay<T>::type, typename std::decay<U>::type>(
		std::forward<T>(first), std::forward<U>(second)
	);
}


/// @brief Hook that allows the sequential_handler to be invoked
///        within specific context based on the hander's type.
template<typename Func, typename Handler>
void asio_handler_invoke(Func f, chained_handler<Handler>* h){
	boost_asio_handler_invoke_helpers::invoke(f, h->_first);
}

/// @brief No operation.
void noop() {}

/// @brief io_service_executor is used to wrap handlers, providing a
///        deferred posting to an io_service.  This allows for chains
///        to inherit io_services from other chains.
class io_service_executor : public std::enable_shared_from_this<io_service_executor>{
public:
	/// @brief Constructor.
	explicit io_service_executor(boost::asio::io_service* ios)
		: _ios(ios)
	{}

	/// @brief Wrap a handler, returning a functor that will post the
	///        provided handler into the io_service.
	///
	/// @param handler Handler to be wrapped for deferred posting.
	/// @return Functor that will post handler into io_service.
	template<class Handler>
	auto wrap(Handler&& h)
		-> decltype(std::bind(&io_service_executor::post<Handler>, shared_from_this(), std::forward<Handler>(h)))
	{
		// By binding to the io_service_exectuer's post, the io_service
		// into which the handler can be posted can be specified at a later 
		// point in time.
		return std::bind(&io_service_executor::post<Handler>, shared_from_this(), std::forward<Handler>(h)));
	}

	/// @brief Set the io_service.
	void io_service(boost::asio::io_service* ios){
		_ios = ios;
	}

	/// @brief Get the io_service.
	boost::asio::io_service* io_service(){
		return _ios;
	}

private:

	/// @brief Post handler into the io_service.
	///
	/// @param handler The handler to post.
	template<class Handler>
	void post(Handler&& h){
		_ios->post(std::forward<Handler>(h));
	}

private:
	boost::asio::io_service* _ios;
};

/// @brief chain_impl is an implementation for a chain.  It is responsible
///        for lifetime management, tracking posting and wrapped functions,
///        as well as determining when run criteria has been satisfied.
class chain_impl : public boost::enable_shared_from_this<chain_impl>{
public:
	/// @brief Constructor.
	chain_impl(std::shared_ptr<io_service_executor> executor, std::size_t required)
		: _executor(executor)
		, _required(required)
	{}

	/// @brief Destructor will invoke all posted handlers.
	~chain_impl(){
		run();
	}

	/// @brief Post a handler that will be posted into the executor 
	///        after run criteria has been satisfied.
	template<class Handler>
	void post(Handler&& h){
		_deferred_handlers.emplace_back(_executor->wrap(std::forward<Handler>(h)));
	}

	/// @brief Wrap a handler, returning a chained_handler.  The returned
	///        handler will notify the impl when it has been invoked.
	template<class Handler>
	auto wrap(Handler&& h)
		-> decltype(make_chained_handler(std::forward<Handler>(h), std::bind(&chain_impl::complete, shared_from_this())))
	{
		return make_chained_handler(std::forward<Handler>(h), std::bind(&chain_impl::complete, shared_from_this()));
	}

	/// @brief Force run of posted handlers.
	void run(){
		boost::unique_lock<boost::mutex> guard(_mutex);
		run(guard);
	}

	/// @brief Get the executor.
	std::shared_ptr<io_service_executor> executor(){
		return _executor;
	}

private:
	/// @brief Completion handler invoked when a wrapped handler has been
	///        invoked.
	void complete(){
		boost::unique_lock<boost::mutex> guard(_mutex);

		// Update tracking.
		if(_required)
			--_required;

		// If criteria has not been met, then return early.
		if(_required)
			return;

		// Otherwise, run the handlers.
		run(guard);    
	}

	/// @brief Run handlers.
	void run(boost::unique_lock<boost::mutex>& guard){
		// While locked, swap handlers into a temporary.
		std::vector<std::function<void()>> handlers;
		std::swap(handlers, _deferred_handlers);

		// Run handlers without mutex.
		guard.unlock();
		for(auto& h : handlers)
			h();
		guard.lock();
	}

private:
	std::shared_ptr<io_service_executor> _executor;
	std::size_t _required;

	boost::mutex _mutex;
	std::vector<std::function<void()>> _deferred_handlers;
};

/// @brief Functor used to wrap and post handlers or chains between two
///        implementations.
struct wrap_and_post{
	wrap_and_post(std::shared_ptr<chain_impl> current, std::shared_ptr<chain_impl> next)
		: _current(current)
		, _next(next)
	{}

	/// @brief Wrap a handler with next, then post into current.
	template<class Handler>
	void operator()(Handler&& h){
		// Wrap the handler with the next implementation, then post into the
		// current.  The wrapped handler will keep next alive, and posting into
		// current will cause next::complete to be invoked when current is ran.
		_current->post(_next->wrap(std::forward<Handler>(h)));
	}

	/// @brief Wrap an entire chain, posting into the current.
	void operator()(chain chain);

private:
	std::shared_ptr<chain_impl> _current;
	std::shared_ptr<chain_impl> _next;
};

} // end namespace detail

/// @brief Used to indicate that the a chain will inherit its service from an
///        outer chain.
class inherit_service_type {};
inherit_service_type inherit_service;

/// @brief Chain represents an asynchronous call chain, allowing the overall
///        chain to be constructed in a verbose and explicit manner.
class chain{
public:
  /// @brief Constructor.
  ///
  /// @param io_service The io_service in which the chain will run.
	explicit chain(boost::asio::io_service& io_service)
		: _impl(std::make_shared<detail::chain_impl>(std::make_shared<detail::io_service_executor>(&io_service), 0))
		, _root_impl(_impl)
	{}

	/// @brief Constructor.  The chain will inherit its io_service from an
	///        outer chain.
	explicit chain(inherit_service_type)
		: _impl(std::make_shared<detail::chain_impl>(std::make_shared<detail::io_service_executor>(static_cast<boost::asio::io_service*>(NULL)), 0))
		, _root_impl(_impl)
	{}

	/// @brief Force run posted handlers.
	void run(){
		_root_impl->run();
	}

	/// @brief Chain link that will complete when the amount of wrapped
	///        handlers is equal to required.
	///
	/// @param required The amount of handlers required to be complete.
	// template <typename T>
	// typename boost::enable_if<boost::is_integral<
	// typename boost::remove_reference<T>::type>, chain>::type
	any(std::size_t required = 1){
		return chain(_root_impl, required);
	}

	/// @brief Chain link that wraps all handlers in container, and will
	///        be complete when the amount of wrapped handlers is equal to
	///        required.
	///
	/// @param Container of handlers to wrap.
	/// @param required The amount of handlers required to be complete.
	template<class Container>
	auto any(const Container& container, std::size_t required = 1)
		-> typename boost::disable_if<boost::is_integral<
			typename boost::remove_reference<Container>::type>, chain
		>::type
	{
		return any(container.begin(), container.end(), required);
	}

	/// @brief Chain link that wraps all handlers in iterator range, and will
	///        be complete when the amount of wrapped handlers is equal to
	///        required.
	///
	/// @param Container of handlers to wrap.
	/// @param required The amount of handlers required to be complete.
	template<class Iterator>
	chain any(Iterator begin, Iterator end, std::size_t required = 1){
		return post(begin, end, required);
	}

	/// @brief Chain link that will complete when the amount of wrapped
	///        handlers is equal to required.
	///
	/// @param required The amount of handlers required to be complete.
	template<class T>
	typename boost::enable_if<boost::is_integral<
		typename boost::remove_reference<T>::type>, chain
	>::type
	all(T required){
		return any<T>(required);
	}

	/// @brief Chain link that wraps all handlers in container, and will
	///        be complete when all wrapped handlers from the container 
	///        have been executed.
	///
	/// @param Container of handlers to wrap.
	template <typename Container>
	typename boost::disable_if<boost::is_integral<
		typename boost::remove_reference<Container>::type>, chain
	>::type
	all(const Container& container){
		return any(container, container.size());
	}

	/// @brief Chain link that wraps all handlers in iterator range, and will
	///        be complete when all wrapped handlers from the iterator range
	///        have been executed.
	///
	/// @param Container of handlers to wrap.
	template<class Iterator>
	chain all(Iterator begin, Iterator end){
		return any(begin, end, std::distance(begin, end));
	}

	/// @brief Chain link that represents a single sequential link.
	template<class Handler>
	chain then(Handler&& h){
		// Create next chain.
		chain next(_root_impl, 1);

		// Wrap handlers from the next chain, and post into the current chain.
		detail::wrap_and_post(_impl, next._impl)(std::forward<Handler>(h));

		return next;
	}

	/// @brief Wrap a handler, returning a chained_handler.
	template<class Handler>
	auto wrap(Handler&& h)
		-> decltype(_impl->wrap(std::forward<Handler>(h)))
	{
		return _impl->wrap(std::forward<Handler>(h));
	}

	/// @brief Set the executor.
	void executor(boost::asio::io_service& io_service){
		_impl->executor()->io_service(&io_service);
	}

	/// @brief Check if this chain should inherit its executor.
	bool inherits_executor(){
		return !_impl->executor()->io_service();
	}

private:

	/// @brief Private constructor used to create links in the chain.
	///
	/// @note All links maintain a handle to the root impl.  When constructing a
	///       chain, this allows for links later in the chain to be stored as
	///       non-temporaries.
	chain(std::shared_ptr<detail::chain_impl> root_impl, std::size_t required)
		: _impl(std::make_shared<detail::chain_impl>(root_impl->executor(), required))
		, _root_impl(root_impl)
	{}

	/// @brief Create a new chain link, wrapping handlers and posting into
	///        the current chain.
	template<class Iterator>
	chain post(Iterator begin, Iterator end, std::size_t required){
		// Create next chain.
		chain next(_root_impl, required);

		// Wrap handlers from the next chain, and post into the current chain.
		std::for_each(begin, end,
			detail::wrap_and_post(_impl, next._impl)
		);
		return next;
	}

private:
	std::shared_ptr<detail::chain_impl> _impl;
	std::shared_ptr<detail::chain_impl> _root_impl;
};

void detail::wrap_and_post::operator()(chain c){
	// If next does not have an executor, then inherit from current.
	if (c.inherits_executor())
		c.executor(*current_->executor()->io_service());

	// When current completes, start the chain.
	current_->post(boost::protect(boost::bind(&chain::run, c)));

	// The next impl needs to be aware of when the chain stops, so
	// wrap a noop and append it to the end of the chain.
	c.then(next_->wrap(&detail::noop));  
}

// Convenience functions.
template<class T, class Handler>
chain async(T& t, Handler&& h){
	return chain(t).then(std::forward<Handler>(h));
}

template<class T, class Container>
chain when_all(T& t, const Container& container){
	return chain(t).all(container);
}

template<class T, class Iterator>
chain when_all(T& t, Iterator begin, Iterator end){
	return chain(t).all(begin, end);
}

template<class T, class Container>
chain when_any(T& t, const Container& container){
	return chain(t).any(container);
}

template<class T, class Iterator>
chain when_any(T& t, Iterator begin, Iterator end){
	return chain(t).any(begin, end);
}
