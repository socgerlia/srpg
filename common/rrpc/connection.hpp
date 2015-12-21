#pragma once

#include <vector>
#include <memory>

#include "rrpc/conversion.hpp"
#include "rrpc/streambuf.hpp"
#include "rrpc/package.hpp"
#include "rrpc/header.hpp"
#include "rrpc/session.hpp"

namespace rrpc{

struct binary_archive{
	using iarchive_type = trivial_binary_iarchive;
	using oarchive_type = trivial_binary_oarchive;
};

template<class Protocol, class Archive>
class connection;

template<class Connection>
struct connection_traits;
template<class Protocol, Archive>
struct connection_traits<connection<Protocol, Archive>>{
	using self_type = connection<Protocol, Archive>;
	using protocol_type = Protocol;
	using iarchive_type = typename Archive::iarchive_type;
	using oarchive_type = typename Archive::oarchive_type;

	using socket_type = typename Protocol::socket;
	using socket_ptr = std::shared_ptr<socket_type>;
};

template<>
class connection<boost::asio::ip::tcp, binary_archive>{
public:
	using traits = connection_traits<connection>;
	using packet_ptr = std::shared_ptr<packet>;

protected:
	traits::socket_ptr _socket;

	// union{
	// 	uint32_t _packageSize;
	// 	uint32_t _packageSizeBuf[1];
	// };
	// streambuf _streamBuf;
	// ipackage_type _ipackage;

	// std::vector<session_type> _sessions;
	union{
		uint32_t _readPackageSize;
		uint32_t _readPackageSizeBuf[1];
	};
	std::streambuf _readStreamBuf;

	union{
		uint32_t _sendPackageSize;
		uint32_t _sendPackageSizeBuf[1];
	};
	std::streambuf _sendStreamBuf;
	std::vector<boost::asio::const_buffer> _sendBuffers;
	std::queue<packet_ptr> _sendQueue;

	// void _send(opackage_ptr package){
	// 	using std::placeholders::_1;
	// 	boost::asio::async_write(_socket, package->data(),
	// 		std::bind(_handleSend, this, _1));
	// }
	// void _handleSend(boost::system::error_code ec, opackage_ptr p){
	// 	if(!ec){
	// 		DM("send ok")
	// 	}
	// 	else{
	// 		DM("send fail")
	// 	}
	// 	p->consume();
	// 	// p->get_stream_buf().consume(p->size());
	// }

	void _listen(){
		DM("listening...")
		using std::placeholders::_1;
		async_read(*_socket, _readPackageSizeBuf,
			std::bind(&self_type::_handleReadPackageSize, this, _1));
	}
	void _handleReadPackageSize(boost::system::error_code ec){
		if(ec)
			return;

		little_to_native(_readPackageSize);
		DM("read packageSize is " << _readPackageSize)

		uint32_t size = _readPackageSize & 0x7fffffff;
		if(size > 0){
			using std::placeholders::_1;
			async_read(*_socket, _readStreamBuf.prepare(size),
				std::bind(&self_type::_handleReadPackage, this, _1));
		}
		else
			_listen();
	}
	void _handleReadPackage(boost::system::error_code ec){
		if(ec)
			return;

		bool isMultiPackets = (_readPackageSize & 0x80000000 ~= 0);
		uint32_t size = _readPackageSize & 0x7fffffff;
		_readStreamBuf.commit(size);

		_listen();
	}

	void _postWrite(){
		size_t packetNum = _sendQueue.size();
		if(packetNum == 0)
			return;

		_sendBuffers.clear();
		_sendBuffers.push_back(boost::asio::buffer(_sendPackageSizeBuf));
		if(packetNum == 1){ // 1 packet
			auto& p = _sendQueue.front();
			_sendBuffers.push_back(p->getStreamBuf());
			_sendPackageSize = p->size();
		}
		else{ // multi-packets
			_sendStreamBuf.comsume(_sendStreamBuf.size());

			trivial_binary_oarchive oar(_sendStreamBuf);
			_sendBuffers.emplace_back();
			oar << _sendQueue.size();

			_sendPackageSize = 0;
			for(auto& p : _sendQueue){
				auto size = p->size();
				oar << size;
				_sendBuffers.push_back(p->getStreamBuf());
				_sendPackageSize += size;
			}
			_sendBuffers[1] = boost::asio::buffer(_sendStreamBuf.data());
			_sendPackageSize += _sendBuffers.size();

			_sendPackageSize &= 0x80000000; // mark as multi-packets
		}
		native_to_little(_sendPackageSize);

		using std::placeholders::_1;
		boost::asio::async_write(*_socket, _sendBuffers,
			std::bind(&self_type::_handleSend, this, _1, /*packetNum*/ 1));
	}

	void _handleSend(boost::system::error_code ec, size_t packetNum){
		if(!ec){
			DM("send ok")
			for(size_t i=0; i<packetNum; ++i){
				auto& p = _sendQueue.front();
				p->consume();
				_sendQueue.pop_front();
			}
			_postWrite();
		}
		else{
			DM("send fail")
			// TODO:
		}
	}

public:
	explicit basic_connection(){
	}

	void setSocket(socket_ptr soc){
		_socket = soc;
	}

	bool isSending() const{ return !_sendQueue.empty(); }

	void send(const packet_ptr& p){
		bool canWrite = !isSending();
		_sendQueue.push_back(p);
		if(canWrite)
			_postWrite();
	}

	void run(){
		_listen();
	}
};

} // end namespace rrpc

