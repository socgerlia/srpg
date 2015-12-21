#pragma once

#include <vector>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/yield.hpp>
#include <boost/lockfree/stack.hpp>
#include <boost/bind.hpp>
#include "coroutine_ex.hpp"

struct datagram;

struct channel{
	int id;

	int send_seq;
	std::vector<datagram> send_queue;

	int recv_seq;
};

struct message_header{
	union{
		struct{
			bool is_reliable : 1;
			bool need_receipt : 1;
			bool is_splited : 1;
			int : 0;
		};
		uint8_t flag;
	};
	int seq;
	int channel;
	int receipt;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version){
		ar & flag;
		if(is_reliable){
			ar & seq;
			ar & channel;
		};
		if(need_receipt)
			ar & receipt;
	}	
};

struct message{
	message_header header;
	std::vector<char> buf;
};

struct datagram_header{
	union{
		struct{
			int : 0;
		};
		uint8_t flag;
	};
	int timestamp;
	int seq;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version){
		ar & flag;
		ar & timestamp;
		ar & seq;
	}

	//
	friend std::ostream& operator<<(std::ostream& os, datagram_header& h){
		return os << "flag = " << (int)h.flag
			<< ", timestamp = " << h.timestamp
			<< ", seq = " << h.seq;
	}
};

struct session{
	typedef boost::function<void(const boost::system::error_code&, size_t size)> send_callback;

	// boost::asio::ip::udp::socket& socket_;
	boost::asio::strand strand_;
	boost::lockfree::stack<datagram*> datagrams_;
	size_t mtu_;

	// boost::function<void(const boost::asio::const_buffers_1& buf, send_callback&&)> async_send; // consider template like below
	template<class ConstBufferSequence, class Handler>
	void async_send(ConstBufferSequence&& buf, Handler&& h){
	}

	void send(boost::shared_ptr<message> msg);
	boost::asio::io_service& get_ios() const;
};

struct datagram : boost::enable_shared_from_this<datagram>{
	typedef datagram self_type;

	session& s;
	boost::asio::deadline_timer timer;
	datagram_header header;
	std::vector<boost::asio::const_buffer> buf;

	int retry_cnt;

	datagram(session& s) : s(s), timer(s.get_ios()){}

	void send(){
		s.async_send(
			boost::asio::buffer(buf),
			s.strand_.wrap(
				boost::bind(&self_type::handle_send, shared_from_this(), _1, _2)));
		timer.async_wait(
			s.strand_.wrap(
				boost::bind(&self_type::handle_time_out, shared_from_this(), _1)));
	}

	void handle_send(const boost::system::error_code& ec, size_t size){
		timer.cancel();
		if(!ec) // ok
			;
		else if(ec == boost::asio::error::message_size) // mtu handle
			;
	}

	void handle_time_out(const boost::system::error_code& ec){
		if(ec == boost::asio::error::operation_aborted)
			return;
		send();
	}
};
