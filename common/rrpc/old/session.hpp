#pragma once

#include <future>

#include <boost/asio.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

// #include "rrpc/connection.hpp"
// #include "rrpc/streambuf.hpp"
#include "rrpc/package.hpp"
#include "rrpc/header.hpp"
#include "rrpc/idl.hpp"
// #include "rrpc/method.hpp"

namespace rrpc{

namespace detail{
	template<class Interface, class Implement>
	struct InterfaceInvoker{
		Implement impl;

		InterfaceInvoker(const Implement& i) : impl(i){}
		InterfaceInvoker(Implement&& i) : impl(std::move(i)){}

		template<class Session>
		void operator()(Session& se, header& h, typename Session::opackage_type& op, typename Session::ipackage_type& ip){
#define RRPC_INVOKE_CASE_METHOD_ID(z, id, data) \
	case id: Interface::_invoke(se, h, op, ip, impl, std::integral_constant<method_id_type, id>()); break;

			switch(h.methodId){
			BOOST_PP_REPEAT(256, RRPC_INVOKE_CASE_METHOD_ID, )
			}

#undef RRPC_INVOKE_CASE_METHOD_ID
		}
	};
} // end namespace detail

template<class Connection>
class session{
public:
	friend Connection;

	using self_type = session;

	using connection_type = Connection;
	using iarchive_type = typename connection_type::iarchive_type;
	using oarchive_type = typename connection_type::oarchive_type;

	using opackage_type = typename connection_type::opackage_type;
	using opackage_ptr = typename connection_type::opackage_ptr;

	using ipackage_type = typename connection_type::ipackage_type;

	template<class T>
	using future_type = std::future<T>;
	template<class T>
	using promise_type = std::promise<T>;

	using invoke_function_type = std::function<void(self_type&, header&, opackage_type&, ipackage_type&)>;
	using response_function_type = std::function<void(self_type&, header& h, ipackage_type& iar)>;

protected:
	connection_type& _connection;
	session_id_type _sessionId;
	bool _enableBatch;

	opackage_ptr _package;

	std::map<interface_id_type, invoke_function_type> _invokeMap;
	std::map<operation_id_type, response_function_type> _waitMap;

	void _endSend(){
		_connection.send(_package);
		_package = nullptr;
	}

	/*
	void _handleSend(const boost::system::error_code& ec){
		_writeStreamBuf.clear();
		_batchWriteBuf.clear();
	}

	void _handleReadHeader(const boost::system::error_code& ec){
		// header
		{
			std::istream is(_readStreamBuf);
			iarchive_type ar(is);
			ar & _readHeader;
			_readStreamBuf.consume(sizeof(header));
		}

		// TODO: check if interface and method are valid
		method_id_type methodId = _readHeader.methodId;
		auto it = _invokeMap.find(methodId);
		if(it == _invokeMap.end())
			listen();
		else
			boost::asio::async_read(_socket, _readStreamBuf.prepare(_readHeader.bodySize), std::bind(&client::_handleReadBody, this, _1));
	}
	void _handleReadBody(const boost::system::error_code& ec){
		// invoke
		{
			std::istream is(_readStreamBuf);
			iarchive_type iar(is);

			// TODO: may be void
			one_time_ostreambuf<> outBuf;
			std::ostream os(outBuf);
			oarchive_type oar(os);

			_invokeMap[_readHeader.methodId](*this, oar, iar);
		}

		_readStreamBuf.consume(_readHeader.bodySize);
		listen();
	}

	//*/

	void _invoke(header& h, opackage_type& op, ipackage_type& ip){
		auto it = _invokeMap.find(h.interfaceId);
		if(it != _invokeMap.end())
			it->second(*this, h, op, ip);
	}
	void _response(header& h, ipackage_type& ip){
		auto it = _waitMap.find(h.operationId);
		if(it != _waitMap.end()){
			it->second(*this, h, ip);
			_waitMap.erase(it);
		}
	}

public:
	session(connection_type& c, session_id_type sessionId)
		: _connection(c)
		, _sessionId(sessionId)
		, _enableBatch(false)
	{
	}

	operation_id_type get_free_operation_id() const{
		operation_id_type ret = 0;
		for(auto& p : _waitMap){
			if(ret == p.first)
				++ret;
			else
				return ret;
		}
		// TODO: error
		return ret;
	}

	operation_id_type begin_packet(interface_id_type iid, method_id_type mid, arity_type arity){
		if(!_package)
			_package.reset(new opackage_type);

		operation_id_type oid = get_free_operation_id(); // TODO: if ret is void, no need to use this
		header h = {
			direction::request,
			_sessionId,
			oid,
			iid,
			mid,
			arity
		};

		_package->begin_packet();
		_package->write(h);

		return oid;
	}

	template<class... Args>
	void write(const Args&... args){
		_package->write(args...);
	}

	void end_packet(){
		_package->end_packet();

		if(!_enableBatch)
			_endSend();
	}

	template<class... Args>
	operation_id_type send(interface_id_type iid, method_id_type mid, const Args&... args){
		operation_id_type oid = begin_packet(iid, mid, sizeof...(Args));
		write(args...);
		end_packet();
		return oid;
	}

	void begin_batch(){
		_enableBatch = true;
	}
	void end_batch(){
		_enableBatch = false;
		_endSend();
	}

	template<class Ret>
	future_type<Ret> wait(operation_id_type oid){
		// if(_waitMap.find(oid) != _waitMap.end()){
		// 	// TODO: error
		// 	DM("oid " << oid << " already exists")
		// 	return {};
		// }

		auto pro = std::make_shared<promise_type<Ret>>();
		_waitMap.emplace(
			oid,
			[pro](self_type& se, header& h, ipackage_type& ip){
				pro->set_value(ip.template read<Ret>());
			}
		);
		return pro->get_future();
	}

	template<class Ret, class... Args>
	typename std::enable_if<
		!std::is_void<Ret>::value,
		future_type<Ret>
	>::type
	/*future_type<Ret>*/ request(interface_id_type iid, method_id_type mid, const Args&... args){
		return wait<Ret>(send(iid, mid, args...));
	}
	template<class Ret, class... Args>
	typename std::enable_if<
		std::is_void<Ret>::value
	>::type
	/*void*/ request(interface_id_type iid, method_id_type mid, const Args&... args){
		send(iid, mid, args...);
	}

	// template<class Ret, class... Args, class Implement> // return Ret
	// /*void*/ auto bind(const method<Ret(Args...)>& f, Implement&& impl)
	// -> typename std::enable_if<
	// 	!std::is_void<Ret>::value &&
	// 	!std::is_void<decltype(impl(std::declval<Args>()...))>::value
	// >::type{
	// 	auto id = f.getId();
	// 	if(_invokeMap.find(id) != _invokeMap.end()){
	// 		// TODO: error
	// 		DM("method[id = " << id << "] already exists")
	// 		return;
	// 	}
	// 	// TODO: C++14 movable lambda
	// 	_invokeMap.emplace(id, [impl](self_type& se, header& h, oarchive_type& oar, iarchive_type& iar){
	// 		h.dir = direction::response;
	// 		oar << h;
	// 		oar << detail::deserialize_and_call<Args...>::invoke(impl, iar);
	// 	});
	// }
	// template<class... Args, class Implement> // return void
	// void bind(const method<void(Args...)>& f, Implement&& impl){
	// 	auto id = f.getId();
	// 	if(_invokeMap.find(id) != _invokeMap.end()){
	// 		// TODO: error
	// 		DM("method[id = " << id << "] already exists")
	// 		return;
	// 	}
	// 	// TODO: C++14 movable lambda
	// 	_invokeMap.emplace(id, [impl](self_type& se, header& h, oarchive_type& oar, iarchive_type& iar){
	// 		detail::deserialize_and_call<Args...>::invoke(impl, iar);
	// 	});
	// }

	template<template<class Session> class Interface, class Implement>
	void bind(Implement&& impl){
		interface_id_type interfaceId = Interface<self_type>::interfaceId;

		_invokeMap.emplace(
			interfaceId,
			detail::InterfaceInvoker<Interface<self_type>, Implement>(std::forward<Implement>(impl))
		);
	}
};

} // end namespace rrpc
