#pragma once

#include <type_traits>
#include <tuple>

#ifdef BOOST_PP_VARIADICS
#  undef BOOST_PP_VARIADICS
#endif
#define BOOST_PP_VARIADICS 1

#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/control.hpp>
#include <boost/preprocessor/facilities/is_empty.hpp>

#include "rrpc/integer_sequence.hpp"

// serialize for tuple
namespace boost{ namespace serialization{

template<int index, int size, class Archive, class Tuple>
typename std::enable_if<
	(index >= size)
>::type
inline /*void*/ serialize_tuple_helper(Archive& ar, Tuple& v){
}
template<int index, int size, class Archive, class Tuple>
typename std::enable_if<
	(index < size)
>::type
inline /*void*/ serialize_tuple_helper(Archive& ar, Tuple& v){
	ar & std::get<index>(v);
	serialize_tuple_helper<index+1, size>(ar, v);
}

template<class Archive, class... Args>
void serialize(Archive& ar, std::tuple<Args...>& v, const unsigned int version){
	serialize_tuple_helper<0, sizeof...(Args)>(ar, v);
}

}} // end namespace boost::serialization

namespace rrpc{

namespace detail{

template<class... Args>
struct ret_helper{
	using type = std::tuple<Args...>;
};
template<class T>
struct ret_helper<T>{
	using type = T;
};
template<>
struct ret_helper<>{
	using type = void;
};

/*
template<class T>
struct to_deserialize_type : std::remove_cv<
	typename std::remove_reference<T>::type
>{};

template<class AscendingIntegers, class... Args>
struct deserialize_and_call_detail;
template<size_t... AscendingIntegers, class... Args>
struct deserialize_and_call_detail<std::integer_sequence<size_t, AscendingIntegers...>, Args...>{
	// function object
	template<class Func, class IArchive, class... ExtArgs>
	static auto invoke(Func&& f, IArchive& iar, ExtArgs&&... exts)
		-> decltype( f(std::declval<Args>()..., std::forward<ExtArgs>(exts)...) )
	{
		std::tuple<Args...> args;
		iar >> args;
		return f(std::move(std::get<AscendingIntegers>(args))..., std::forward<ExtArgs>(exts)...);
	}

	// member function
	template<class Self, class Func, class IArchive, class... ExtArgs>
	static auto invoke(Self& p, const Func& f, IArchive& iar, ExtArgs&&... exts)
		-> typename std::enable_if<
			std::is_member_pointer<Func>::value,
			decltype( (p.*f)(std::declval<Args>()..., std::forward<ExtArgs>(exts)...) )
		>::type
	{
		std::tuple<Args...> args;
		iar >> args;
		return (p.*f)(std::move(std::get<AscendingIntegers>(args))..., std::forward<ExtArgs>(exts)...);
	}
};
template<>
struct deserialize_and_call_detail<std::integer_sequence<size_t>>{
	// function object
	template<class Func, class IArchive, class... ExtArgs>
	static auto invoke(Func&& f, IArchive& iar, ExtArgs&&... exts)
		-> decltype( f(std::forward<ExtArgs>(exts)...) )
	{
		return f(std::forward<ExtArgs>(exts)...);
	}

	// member function
	template<class Self, class Func, class IArchive, class... ExtArgs>
	static auto invoke(Self& p, const Func& f, IArchive& iar, ExtArgs&&... exts)
		-> typename std::enable_if<
			std::is_member_pointer<Func>::value,
			decltype( (p.*f)(std::forward<ExtArgs>(exts)...) )
		>::type
	{
		return (p.*f)(std::forward<ExtArgs>(exts)...);
	}
};

template<class... Args>
struct deserialize_and_call : deserialize_and_call_detail<
	std::make_index_sequence<sizeof...(Args)>,
	typename to_deserialize_type<Args>::type...
>{};

template<class... Args, class IArchive>
std::tuple<typename to_deserialize_type<Args>::type...> read_args(IArchive& iar){
	std::tuple<typename to_deserialize_type<Args>::type...> ret;
	iar >> ret;
	return ret;
};
//*/

} // end namespace detail

#define RRPC_EMPTY(x)
#define RRPC_IS_TUPLE_EMPTY(t) BOOST_PP_IS_EMPTY(BOOST_PP_TUPLE_ELEM(0, t))
#define RRPC_ENABLE_IF_NOT_EMPTY(args_tuple, macro) \
	BOOST_PP_IIF( RRPC_IS_TUPLE_EMPTY(args_tuple), \
		RRPC_EMPTY, \
		macro \
	)(args_tuple)

#define RRPC_ARG_TYPE_NAME(r, data, i, elm) BOOST_PP_COMMA_IF(i) elm data##i
#define RRPC_ARGS_TYPE_NAME_(args_tuple) BOOST_PP_SEQ_FOR_EACH_I(RRPC_ARG_TYPE_NAME, arg, BOOST_PP_TUPLE_TO_SEQ(args_tuple))
#define RRPC_ARGS_TYPE_NAME(args_tuple) RRPC_ENABLE_IF_NOT_EMPTY(args_tuple, RRPC_ARGS_TYPE_NAME_)

#define RRPC_COMMA_ARG_NAME(r, data, i, elm) , data##i
#define RRPC_COMMA_ARGS_NAME_(args_tuple) BOOST_PP_SEQ_FOR_EACH_I(RRPC_COMMA_ARG_NAME, arg, BOOST_PP_TUPLE_TO_SEQ(args_tuple))
#define RRPC_COMMA_ARGS_NAME(args_tuple) RRPC_ENABLE_IF_NOT_EMPTY(args_tuple, RRPC_COMMA_ARGS_NAME_)

#define RRPC_ARG_DECLVAL(r, data, i, elm) BOOST_PP_COMMA_IF(i) std::declval<elm>()
#define RRPC_ARGS_DECLVAL_(args_tuple) BOOST_PP_SEQ_FOR_EACH_I(RRPC_ARG_DECLVAL, args, BOOST_PP_TUPLE_TO_SEQ(args_tuple))
#define RRPC_ARGS_DECLVAL(args_tuple) RRPC_ENABLE_IF_NOT_EMPTY(args_tuple, RRPC_ARGS_DECLVAL_)

#define RRPC_ARG_TUPLE_GET(r, data, i, elm) BOOST_PP_COMMA_IF(i) std::get<i>(data)
#define RRPC_ARGS_TUPLE_GET_(args_tuple) BOOST_PP_SEQ_FOR_EACH_I(RRPC_ARG_TUPLE_GET, args, BOOST_PP_TUPLE_TO_SEQ(args_tuple))
#define RRPC_ARGS_TUPLE_GET(args_tuple) RRPC_ENABLE_IF_NOT_EMPTY(args_tuple, RRPC_ARGS_TUPLE_GET_)

// #define INTERFACE_RETURN_VOID() std::is_void<ret>::value
#define RRPC_IMPL_RETURN_VOID(funcname, args_tuple) std::is_void<decltype(impl.funcname(RRPC_ARGS_DECLVAL(args_tuple)))>::value
#define RRPC_IMPL_PROMISE_RETURN_VOID(funcname, args_tuple) std::is_void<decltype(impl.funcname(RRPC_ARGS_DECLVAL(args_tuple), std::declval<typename Session::promise_type<ret>&>()))>::value

#define RRPC_METHOD_RET_TYPE(rets_tuple) \
	typename rrpc::detail::ret_helper<BOOST_PP_TUPLE_REM_CTOR(rets_tuple)>::type

#define RRPC_METHOD_DETAIL(ret, funcname, args_tuple, id) \
	public:																									\
		auto funcname(RRPC_ARGS_TYPE_NAME(args_tuple)) 														\
			-> decltype(_session.template request<ret>(interfaceId, id  RRPC_COMMA_ARGS_NAME(args_tuple))) 	\
		{ 																									\
			return _session.template request<ret>(interfaceId, id  RRPC_COMMA_ARGS_NAME(args_tuple));		\
		}																									\
		template<class Implement>																			\
		static /*void*/ auto _invoke(Session& se, rrpc::header& h, opackage_type& op, ipackage_type& ip, Implement& impl, std::integral_constant<rrpc::method_id_type, id>) \
			-> typename std::enable_if<																		\
				RRPC_IMPL_RETURN_VOID(funcname, args_tuple) && std::is_void<ret>::value						\
			>::type																							\
		{																									\
			auto args = ip.template read_args<BOOST_PP_TUPLE_REM_CTOR(args_tuple)>();						\
			impl.funcname(RRPC_ARGS_TUPLE_GET(args_tuple));													\
		}																									\
		template<class Implement>																			\
		static /*void*/ auto _invoke(Session& se, rrpc::header& h, opackage_type& op, ipackage_type& ip, Implement& impl, std::integral_constant<rrpc::method_id_type, id>) \
			-> typename std::enable_if<																		\
				!RRPC_IMPL_RETURN_VOID(funcname, args_tuple) && !std::is_void<ret>::value					\
			>::type																							\
		{																									\
			op.begin_packet();																				\
			h.dir = rrpc::direction::response;																\
			op.write(h);																					\
			auto args = ip.template read_args<BOOST_PP_TUPLE_REM_CTOR(args_tuple)>();						\
			op.write(impl.funcname(RRPC_ARGS_TUPLE_GET(args_tuple)));										\
			op.end_packet();																				\
		}																									
		// template<class Implement> /*continuation*/															\
		// static /*void*/ auto _invoke(Session& se, rrpc::header& h, oarchive_type& oar, iarchive_type& iar, Implement& impl, std::integral_constant<rrpc::method_id_type, id>) \
		// 	-> typename std::enable_if<																		\
		// 		RRPC_IMPL_PROMISE_RETURN_VOID(funcname, args_tuple) && !std::is_void<ret>::value			\
		// 	>::type																							\
		// {																									\
		// 	auto args = rrpc::detail::read_args<BOOST_PP_TUPLE_REM_CTOR(args_tuple)>(iar);								\
		// 	impl.funcname(																			\
		// 		RRPC_ARGS_TUPLE_GET(args_tuple) BOOST_PP_COMMA_IF(1 - RRPC_IS_TUPLE_EMPTY(args_tuple))		\
		// 		make_continuation(se, h)); 																	\
		// }

#define RRPC_INTERFACE_BEGIN(name, iid) 						\
	template<class Session> 									\
	class name{ 												\
	public:														\
		enum{ baseId = __COUNTER__+1 }; 						\
		enum{ interfaceId = iid }; 								\
		using self_type = name; 								\
		using session_type = Session;							\
		using oarchive_type = typename Session::oarchive_type; 	\
		using iarchive_type = typename Session::iarchive_type; 	\
		using opackage_type = typename rrpc::package<oarchive_type>; 			\
		using ipackage_type = typename rrpc::ipackage<iarchive_type>; 			\
																\
	protected:													\
		session_type& _session;									\
		rrpc::interface_id_type _interfaceId;					\
																\
	public:														\
		name(session_type& se) : _session(se){}					\
																\
		template<class Implement, rrpc::method_id_type id>		\
		static void _invoke(Session& se, rrpc::header& h, opackage_type& op, ipackage_type& ip, Implement& impl, std::integral_constant<rrpc::method_id_type, id>){ \
			DM("please implement the methodId = " << h.methodId) \
		}

#define RRPC_INTERFACE_END(name) \
	};

#define RRPC_METHOD(funcname, rets_tuple, args_tuple) \
	RRPC_METHOD_DETAIL(RRPC_METHOD_RET_TYPE(rets_tuple), funcname, args_tuple, (__COUNTER__ - baseId))


template<template<class Session> class Interface, class Session>
Interface<Session> make_interface(Session& se){
	return {se};
}

} // end namespace rrpc
