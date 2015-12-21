#pragma once

#include <type_traits>
#include <boost/endian/conversion.hpp>
#include <boost/iostreams/put.hpp>
#include <boost/iostreams/write.hpp>
#include "rrpc/fixed.hpp"
#include "mpl/make_integer_type.hpp"

namespace RR{
namespace rrpc{

template<class Sink, class T>
void encode(Sink& snk, const T& v);

namespace detail{
namespace io = boost::iostreams;

template<class T, class Enable = void>
struct encoder : std::integral_constant<bool, false>{};
	// template<class Sink>
	// static void invoke(Sink& ar, const T& v){
	// 	boost::serialization::serialize(ar, const_cast<T&>(v), boost::serialization::version<T>::value);
	// }

// // enum
// template<class T>
// struct encoder<T, typename std::enable_if<
// 	std::is_enum<T>::value
// >::type>{
// 	template<class Archive>
// 	static void invoke(Archive& ar, T v){
// 		using integer_type = typename std::underlying_type<T>::type;
// 		encode(ar, static_cast<integer_type>(v));
// 	}
// };

// var unsigned integer
template<class T>
struct encoder<T, typename std::enable_if<
	std::is_integral<T>::value && std::is_unsigned<T>::value
>::type> : std::integral_constant<bool, true>
{
	template<class Sink>
	static void invoke(Sink& snk, T v){
		const unsigned int shift = 7;
		uint8_t byte;
		for(;;){
			byte = static_cast<uint8_t>(v & 0x7f);
			v >>= shift;
			if(v == 0){
				io::put(snk, byte);
				break;
			}
			else{
				byte |= 0x80;
				io::put(snk, byte);
			}
		}
	}
};

// var signed integer
template<class T>
struct encoder<T, typename std::enable_if<
	std::is_integral<T>::value && std::is_signed<T>::value
>::type> : std::integral_constant<bool, true>
{
	template<class Sink>
	static void invoke(Sink& snk, T v){
		using UT = typename std::make_unsigned<T>::type;
		const unsigned int bits = sizeof(T) * 8;
		encode(snk, static_cast<UT>((v << 1) ^ (v >> (bits - 1))));
	}
};

// fixed
template<class T>
struct encoder<fixed<T>> : std::integral_constant<bool, true>
{
	template<class Sink>
	static void invoke(Sink& snk, fixed<T> v){
		boost::endian::native_to_little_inplace(v.value);
		io::write(snk, reinterpret_cast<char*>(&v.value), sizeof(v.value));
	}
};

// float point
template<class T>
struct encoder<T, typename std::enable_if<
	std::is_floating_point<T>::value
>::type> : std::integral_constant<bool, true>
{
	template<class Sink>
	static void invoke(Sink& snk, T v){
		encode(snk, reinterpret_cast<fixed<typename make_integer_type<sizeof(T)>::type>&>(v));
	}
};

} // end namespace detail

template<class Sink, class T>
inline void encode(Sink& snk, const T& v){
	detail::encoder<T>::invoke(snk, v);
}

template<class T>
bool can_encode = detail::encoder<T>::value;

} // end namespace rrpc
} // end namespace RR
