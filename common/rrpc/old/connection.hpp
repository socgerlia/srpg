#pragma once

#include <vector>
#include <memory>

#include "rrpc/conversion.hpp"
#include "rrpc/streambuf.hpp"
#include "rrpc/package.hpp"
#include "rrpc/header.hpp"
#include "rrpc/session.hpp"

namespace rrpc{

// template<class Connection>
// class session;

template<class ArchiveType>
class basic_connection{
public:
	using self_type = basic_connection;
	using session_type = session<self_type>;

	using socket_type = boost::asio::ip::tcp::socket;
	using socket_ptr = std::shared_ptr<socket_type>;

	using iarchive_type = typename ArchiveType::iarchive_type;
	using oarchive_type = typename ArchiveType::oarchive_type;

	using opackage_type = package<oarchive_type>;
	using opackage_ptr = std::shared_ptr<opackage_type>;

	using ipackage_type = ipackage<iarchive_type>;

protected:
	socket_ptr _socket;

	// union{
	// 	uint32_t _packageSize;
	// 	uint32_t _packageSizeBuf[1];
	// };
	// streambuf _streamBuf;
	ipackage_type _ipackage;

	std::vector<session_type> _sessions;

	// void _send(opackage_ptr package){
	// 	using std::placeholders::_1;
	// 	boost::asio::async_write(_socket, package->data(),
	// 		std::bind(_handleSend, this, _1));
	// }
	void _handleSend(boost::system::error_code ec, opackage_ptr p){
		if(!ec){
			DM("send ok")
		}
		else{
			DM("send fail")
		}
		p->consume();
		// p->get_stream_buf().consume(p->size());
	}

	void _listen(){
		DM("listening...")
		using std::placeholders::_1;
		async_read(*_socket, _ipackage.get_package_size_buf(),
			std::bind(&self_type::_handleReadPackageSize, this, _1));
	}
	void _handleReadPackageSize(boost::system::error_code ec){
		if(ec)
			return;

		_ipackage.unpack_package_size();
		auto size = _ipackage.size();
		DM("read packageSize is " << size)

		if(size > 0){
			using std::placeholders::_1;
			async_read(*_socket, _ipackage.prepare(),
				std::bind(&self_type::_handleReadPackage, this, _1));
		}
		else
			_listen();
	}
	void _handleReadPackage(boost::system::error_code ec){
		if(ec)
			return;

		auto& ip = _ipackage;

		ip.commit();
		// _streamBuf.commit(_packageSize);
		// iarchive_type iar(_streamBuf);

		// get packet size list
		auto& packetSizeList = ip.unpack_packet_sizes();
		DM("packet size list: {")
		for(auto packetSize : packetSizeList){
			DM(packetSize << ", ")
		}
		DM("}")

		// invoke
		opackage_ptr pop = std::make_shared<opackage_type>();
		auto& op = *pop;

		header h;
		uint32_t packet_in_avail;
		int bytes_left;

		for(auto packetSize : packetSizeList){
			packet_in_avail = ip.in_avail();
			if(packet_in_avail < packetSize)
				break; // error

			ip.read_header(h);
			DM("read header: " << h)

			if(h.sessionId < _sessions.size()){
				switch(h.dir){
				case direction::request:
					_sessions[h.sessionId]._invoke(h, op, ip); break;
				case direction::response:
					_sessions[h.sessionId]._response(h, ip); break;
				default:
					break;
				}
			}

			// discard left bytes of this packet
			bytes_left = packetSize - (packet_in_avail - ip.in_avail());
			if(bytes_left == 0)
				;
			else if(bytes_left > 0)
				ip.ignore(bytes_left);
			else
				break; // error
		}

		// discard left bytes
		bytes_left = ip.in_avail();
		DM("still in_avail = " << bytes_left)
		if(bytes_left > 0)
			ip.ignore(bytes_left);

		if(op.size() > 0)
			send(std::move(pop));

		_listen();
	}

public:
	explicit basic_connection(){
	}

	void setSocket(socket_ptr soc){
		_socket = soc;
	}

	void send(opackage_ptr package){
		package->pack();
		using std::placeholders::_1;
		boost::asio::async_write(*_socket, package->data(),
			std::bind(&self_type::_handleSend, this, _1, package));
		DM("sending... size: " << package->size())
	}

	void run(){
		_listen();
	}

	session_type& make_session(){
		_sessions.emplace_back(*this, static_cast<uint8_t>(_sessions.size()));
		return _sessions.back();
	}
};

} // end namespace rrpc

