#pragma once

namespace rrpc{

class endpoint_manager{
	using buffer_type = boost::asio::mutable_buffer;

public:
	template<class Handler>
	void async_send(endpoint& src, endpoint& dst, buffer_type buf, Handler&& h){
		
	}
};

}
