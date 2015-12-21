#pragma once

#include <functional>
#include "session.hpp"

namespace rrpc{

template<class T>
class future{
	session& _session;
	uint16_t _operationId;

	future(const future&) = delete;
	future& operator=(const future&) = delete;

public:
	future(session& s)
		: _session(s)
	{}

	~future(){
		wait();
	}

	void wait(){
		// block...
	}
	T get(){
		wait();
	}

	template<class Func>
	auto then(Func&& f)
		-> future<decltype(f(*this))>
	{
	}
};

// specialization

// void
template<>
future<void>::~future(){
}

} // end namespace rrpc
