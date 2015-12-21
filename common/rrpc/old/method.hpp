#pragma once

// #include "rrpc/future.hpp"
#include "rrpc/package.hpp"
#include "rrpc/header.hpp"

namespace rrpc{

template<class Signature>
struct method;

template<class Ret, class... Args>
struct method<Ret(Args...)>{
	using result_type = Ret;

	method_id_type _id;

public:
	method(method_id_type id)
		: _id(id)
	{
	}

	method_id_type getId() const{
		return _id;
	}

	/*
	// the same as request
	future<Ret> operator()(session& se, const Args&... args){
		return request(se, args...);
	}
	template<class Func>
	void operator()(session& se, const Args&... args, Func&& f){
		request(se, args..., std::forward<Func>(f));
	}
	//*/

	// template<class Session>
	// void operator()(Session& se, const Args&... args){
	// 	se.send(_id, args...);
	// }
	template<class Session>
	auto request(Session& se, const Args&... args) const
		-> decltype(se.template request<Ret>(getId(), args...))
	{
		return se.template request<Ret>(-1, getId(), args...);
	}
	template<class Session>
	auto operator()(Session& se, const Args&... args) const
		-> decltype(request(se, args...))		
	{
		return request(se, args...);
	}

	// template<class Session>
	// future<Ret> request(Session& se, const Args&... args){
	// 	return se.send(id, args...);
	// }
	template<class Archive, class Implement>
	void response(Archive& ar){
	}

	// template<class Session, class Implement>
	// Ret invoke(Session& se, Implement&& impl, typename Session::iarchive_type& iar) const{
	// 	return detail::deserialize_and_call<Ret, Args...>(std::forward<Implement>(impl), iar);
	// }
	// template<class Session, class Implement, class Continuation>
	// void invoke(Session& se, Implement&& impl, typename Session::iarchive_type& iar, Continuation&& cc) const{
	// 	detail::deserialize_and_call<void, Args...>(std::forward<Implement>(impl), iar);
	// }

	template<class Session>
	void wait(Session& se, operation_id_type operationId){
	}
};

} // end namespace rrpc
