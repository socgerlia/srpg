#include <iostream>
#include <functional>

using namespace std;

#define DM(x) std::cout << x << std::endl;

void* lua_getstate(){
	return nullptr;
}
void lua_pushfunction(void*, int){
	DM("lua_pushfunction")
}
template<class... Args>
void lua_push(void*, Args&&... args){
	DM("lua_push")
}
void lua_call(void*, int, int){
	DM("lua_call")
}
void lua_pop(void*, int){
	DM("lua_pop")
}
template<class T>
T lua_read(void*, int){
	DM("lua_read")
	return T();
}

template<class Signature>
class lua_func;
template<class Ret, class... Args>
class lua_func<Ret(Args...)>{
	int _ref;

public:
	Ret operator()(Args&&... args){
		auto L = lua_getstate();
		lua_pushfunction(L, _ref);
		lua_push(L, std::forward<Args>(args)...);
		lua_call(L, sizeof...(Args), 1);

		struct defer{ ~defer(){
			auto L = lua_getstate();
			lua_pop(L, 1);
		}}_;
		return lua_read<Ret>(L, -1);
	}
};
template<class... Args>
class lua_func<void(Args...)>{
	int _ref;

public:
	void operator()(Args&&... args){
		auto L = lua_getstate();
		lua_pushfunction(L, _ref);
		lua_push(L, std::forward<Args>(args)...);
		lua_call(L, sizeof...(Args), 0);
	}
};

// template<class Ret, class... Args>
// typename std::enable_if<
// 	!std::is_void<Ret>::value,
// 	Ret
// >::type
// /*Ret*/ lua_func<Ret(Args...)>::operator()(Args&&... args){
// 	auto L = lua_getstate();
// 	lua_pushfunction(L, _ref);
// 	lua_push(L, std::forward<Args>(args)...);
// 	lua_call(L, sizeof...(Args), 1);

// 	struct defer{ ~defer(){
// 		auto L = lua_getstate();
// 		lua_pop(L, 1);
// 	}}_;
// 	return lua_read<Ret>(L, -1);
// }

// template<class Ret, class... Args>
// typename std::enable_if<
// 	std::is_void<Ret>::value
// 	// Ret
// >::type
// /*Ret*/ lua_func<Ret(Args...)>::operator()(Args&&... args){
// 	auto L = lua_getstate();
// 	lua_pushfunction(L, _ref);
// 	lua_push(L, std::forward<Args>(args)...);
// 	lua_call(L, sizeof...(Args), 0);
// }

std::function<void(float)> global_function;

template<class Func>
void set_callback(Func&& f){
	global_function = std::move(f);
}

int main(int argc, char** argv){
	// std::function<float(float)> f = lua_func<float(float)>();
	set_callback(lua_func<void(float)>());
	global_function(12);

	lua_func<float(float, float)>()(123, 123);
	return 0;
}
