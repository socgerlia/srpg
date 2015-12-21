#pragma once

#include <type_traits>
#include "mpl/copy_cvr.hpp"

namespace RR{
namespace rrpc{

template<class T>
struct fixed{
	static_assert(std::is_integral<T>::value, "integer type expected");
	T value;

	operator T&(){
		return value;
	}
};

template<class Archive, class T>
void serialize(Archive& ar, fixed<T>& v, const unsigned int version){
	ar & v.value;
}

template<class T, class U>
fixed<T> make_fixed(U v){
	return { static_cast<T>(v) };
}

template<class T>
inline constexpr decltype(auto) fixed_cast(T&& v){
	using base_type = typename std::decay<T>::type;
	using result_type = typename copy_cvr<T&&, fixed<base_type>>::type;
	return reinterpret_cast<result_type>(v);
}

} // end namespace rrpc
} // end namespace RR
