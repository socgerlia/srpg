#pragma once

namespace rrpc{

class endpoint{
public:
	using receive_handler = std::function<void(endpoint&, /*data*/)>;

protected:
	endpoint_manager& _manager;
	receive_handler _receive_callback;

public:
	template<class Handler>
	void set_receive_callback(Handler&& h){		
		_receive_callback = std::forward<Handler>(h);
	}

	template<class Handler>
	void async_check_reachable(endpoint& dst, Handler&& h);

	template<class Handler>
	void async_send(endpoint& dst, Handler&& h);

	template<class Iter, class Handler>
	void async_send_multi(Iter b, Iter e, Handler&& h);

	void async_find_endpoint();
};

}
