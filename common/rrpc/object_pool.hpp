#pragma once

#include <queue>

namespace rrpc{

template<class T, bool ThreadSafe = true>
class object_pool{
protected:
	boost::lockfree::queue<T*> queue_;

public:
	~object_pool(){
		queue_.consume_all([](T* p){ delete p; });
	}

	template<class... Args>
	T* construct(Args&&... args){
		T* p = nullptr;
		return queue_.pop(p) ? p : new T(std::forward<Args>(args)...);
	}

	void destruct(T* p){
		if(!queue_.push(p))
			delete p;
	}

	template<class... Args>
	boost::shared_ptr<T> construct_with_auto_destruct(Args&&... args){
		return boost::shared_ptr<T>(
			construct(std::forward<Args>(args)...),
			[this](T* p){ destruct(p); }
		);
	}
};

template<class T, bool ThreadSafe = true>
class object_pool_strong{
protected:
	boost::shared_ptr<object_pool<T, ThreadSafe>> inner_;

public:
	object_pool_strong() : inner_(boost::make_shared<object_pool<T, ThreadSafe>>()){}

	template<class... Args>
	T* construct(Args&&... args){
		return inner_->construct(std::forward<Args>(args)...);
	}

	void destruct(T* p){
		inner_->destruct(p);
	}

	template<class... Args>
	boost::shared_ptr<T> construct_with_auto_destruct(Args&&... args){
		return boost::shared_ptr<T>(
			construct(std::forward<Args>(args)...),
			[wp = boost::weak_ptr<T>{inner_}](T* p){
				auto inner = wp.lock();
				if(inner)
					inner->destruct(p);
				else
					delete p;
			}
		);
	}
};

}
