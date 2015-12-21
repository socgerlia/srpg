#pragma once

#include <utility>
#include <tuple>

#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/control.hpp>
#include <boost/preprocessor/facilities/is_empty.hpp>

template<class...> struct void_t{ using type = void; };

template<int Id, class Signature, class... Options>
struct basic_method_info;

template<int Id, class Ret, class... Args, class... Options>
struct basic_method_info<Id, Ret(Args...), Options...>{
	static const int id = Id;
	using return_type = Ret;
	using args_type = std::tuple<Args...>;
	using options = std::tuple<Options...>;
};

struct get_method_info{};

template<class Interface, class Proxy>
class interface_client;

template<class Interface, class Proxy, class Impl>
class interface_server : Interface{
protected:
	Proxy proxy_;
	Impl& impl_;

public:
	// template<int id> auto invoke(std::integral_constant<int, id>){
	// 	return method_info<id>::invoke(impl_);
	// }
};

template<class Interface, class Proxy, class Impl>
void bind_server(Proxy proxy, Impl& impl){

}

// detail
#define _RRPC_APPLY(macro, tuple) macro tuple
#define _RRPC_WRAP(...) (__VA_ARGS__)

#define _RRPC_SEQ_WRAP(seq) BOOST_PP_CAT(_RRPC_SEQ_WRAP_REC_0 seq, _END)
#define _RRPC_SEQ_WRAP_REC_0(...) ((__VA_ARGS__))_RRPC_SEQ_WRAP_REC_1
#define _RRPC_SEQ_WRAP_REC_1(...) ((__VA_ARGS__))_RRPC_SEQ_WRAP_REC_0
#define _RRPC_SEQ_WRAP_REC_0_END
#define _RRPC_SEQ_WRAP_REC_1_END

#define _RRPC_IS_TUPLE_EMPTY(tuple) BOOST_PP_EXPAND(BOOST_PP_IS_EMPTY tuple)
#define _RRPC_IS_TUPLE_NOT_EMPTY(tuple) BOOST_PP_NOT(_RRPC_IS_TUPLE_EMPTY(tuple))
#define _RRPC_TUPLE_ENABLE_IF(tuple, macro) \
	BOOST_PP_IIF(							\
		_RRPC_IS_TUPLE_EMPTY(tuple),		\
		BOOST_PP_TUPLE_EAT(), 				\
		macro 								\
	)

#define _RRPC_TUPLE_FOR_EACH_I(macro, data, tuple) \
	_RRPC_TUPLE_ENABLE_IF(tuple, BOOST_PP_LIST_FOR_EACH_I)(macro, data, BOOST_PP_TUPLE_TO_LIST(tuple))

#define _RRPC_COMMA_ARG_TYPE_NAME_ITER(r, data, i, elem) , elem BOOST_PP_CAT(arg, i)
#define _RRPC_ARG_NAME_ITER(r, data, i, elem) BOOST_PP_COMMA_IF(i) BOOST_PP_CAT(arg, i)

#define _RRPC_DECLVAL_ARG_ITER(r, data, i, elem) BOOST_PP_COMMA_IF(i) std::declval<elem>()

#define _RRPC_METHOD_INFO_ITER(r, data, i, elem) \
	_RRPC_TUPLE_ENABLE_IF(elem, _RRPC_APPLY)(_RRPC_METHOD_INFO, (i, BOOST_PP_REM elem))
#define _RRPC_METHOD_INFO(id, funcname, ret, args_tuple, ...) 									\
	static method_info<id> funcname(get_method_info);											\
	template<class dummy>																		\
	struct method_info<id, dummy> : basic_method_info<											\
		id,																						\
		ret args_tuple																			\
		BOOST_PP_COMMA_IF(_RRPC_IS_TUPLE_NOT_EMPTY((__VA_ARGS__))) __VA_ARGS__					\
	>{ 																							\
		template<class Impl, class Enable = void> struct match_signature : std::false_type{}; 	\
		template<class Impl> struct match_signature<Impl, typename void_t<						\
			decltype(std::declval<Impl&>().funcname(_RRPC_TUPLE_FOR_EACH_I(_RRPC_DECLVAL_ARG_ITER, _, args_tuple))) \
		>::type> : std::true_type{};															\
																								\
		template<class Impl, class... Args>														\
		static auto invoke(Impl& impl, Args&&... args){ 										\
			static_assert(																		\
				match_signature<Impl>::value,													\
				"the signature of " #funcname " does not meet requirement"); 					\
			return impl.funcname(std::forward<Args>(args)...);									\
		}																						\
	};

#define _RRPC_METHOD_CALL_ITER(r, data, i, elem) \
	_RRPC_TUPLE_ENABLE_IF(elem, _RRPC_APPLY)(_RRPC_METHOD_CALL, (i, BOOST_PP_REM elem))
#define _RRPC_METHOD_CALL(id, funcname, ret, args_tuple, ...) \
	template<class... Args> auto funcname(Args&&... args){ return proxy_.template request<method_info<id>>(std::forward<Args>(args)...); }

#define _RRPC_INTERFACE(interface_name, methods) 											\
	struct interface_name{ 																	\
		template<int Id, class dummy = void> struct method_info;							\
		BOOST_PP_SEQ_FOR_EACH_I(_RRPC_METHOD_INFO_ITER, _, methods)							\
	};																						\
																							\
	template<class Proxy>																	\
	class interface_client<interface_name, Proxy> : interface_name{							\
	protected:																				\
		Proxy proxy_;																		\
																							\
	public:																					\
		interface_client(Proxy proxy) : proxy_(proxy){}										\
		BOOST_PP_SEQ_FOR_EACH_I(_RRPC_METHOD_CALL_ITER, _, methods)							\
	};

// struct tcp_client{
// 	template<class Info>
// 	struct get_invoker{
// 		template<class Stub, class... Args>
// 		static auto invoke(Stub& stub, Args&&... args){
// 			auto op = stub.begin_packet();
// 			stub.write(args...);
// 			stub.end_packet();
// 			return stub.wait(op);
// 		}
// 	};
// };

// // template<class Stub>
// // class interface_server : interface{
// // protected:
// // 	Stub& stub_;

// // public:
// // 	template<int id> auto invoke(Stub& stub, std::integral_constant<int, id>){ return info<id>::invoke(stub_, reader); }
// // };

// struct tcp_server{
// 	template<class Info>
// 	struct get_invoker{
// 		template<class Stub, class... Args>
// 		static auto invoke(Stub& stub, Args&&... args){
// 			auto op = stub.begin_packet();
// 			stub.write(args...);
// 			stub.end_packet();
// 			return stub.wait(op);
// 		}
// 	};
// };

template<class Interface, class Proxy>
interface_client<Interface, typename std::decay<Proxy>::type> make_client(Proxy&& proxy){
	return { proxy };
}

#define RRPC_GET_METHOD_INFO(interface_name, funcname) decltype(interface_name::funcname(get_method_info()))
#define RRPC_INTERFACE(interface_name, methods) _RRPC_INTERFACE(interface_name, _RRPC_SEQ_WRAP(methods))

// _RRPC_SEQ_WRAP((echo, void, int)(echo, void, int))
// _RRPC_TUPLE_FOR_EACH_I(_RRPC_COMMA_ARG_TYPE_NAME_ITER, _, (int, int, string))

#include "options.hpp"

RRPC_INTERFACE(
	test_interface,
	(echo, std::string, (std::string), rrpc::options::compress)
	()
	(get_time, int, ())
	(set_pos, void, (float, float))
)


