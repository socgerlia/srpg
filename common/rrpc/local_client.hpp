#pragma once

// namespace rrpc{

namespace detail{
template<class Impl>
struct local_client_proxy{
	Impl& impl_;

	template<class Info, class... Args>
	auto request(Args&&... args){
		return Info::invoke(impl_, std::forward<Args>(args)...);
	}
};
}

template<class Impl>
::detail::local_client_proxy<Impl> local_client(Impl& impl){
	return { impl };
}

// }
