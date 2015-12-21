#pragma once

#include <type_traits>
#include <boost/endian/conversion.hpp>
#include <boost/iostreams/get.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>
#include "fixed.hpp"

namespace rrpc{

template<class T, class Archive>
void decode(Archive& ar, T& v);

}

namespace rrpc{ namespace detail{

namespace io = boost::iostreams;

template<class T, class Enable=void>
struct decoder{
	template<class Archive>
	static void invoke(Archive& ar, T& v){
		boost::serialization::serialize(ar, v, boost::serialization::version<T>::value);
	}
};

// enum
template<class T>
struct decoder<T, typename std::enable_if<
	std::is_enum<T>::value
>::type>{
	template<class Archive>
	static void invoke(Archive& ar, T& v){
		using integer_type = typename std::underlying_type<T>::type;
		decode(ar, *reinterpret_cast<integer_type*>(&v));
	}
};

// var unsigned integer
template<class T>
struct decoder<T, typename std::enable_if<
	std::is_integral<T>::value && std::is_unsigned<T>::value
>::type>{
	template<class Archive>
	static void invoke(Archive& ar, T& v){
		const unsigned int dshift = 7;
		unsigned int shift = 0;
		int byte;

		v = 0;
		for(;;){
			byte = io::get(ar.src_); // TODO: eof
			v |= (T(byte & 0x7f) << shift);
			if(byte < 0x80)
				break;
			shift += dshift;
		}
	}
};

// var signed integer
template<class T>
struct decoder<T, typename std::enable_if<
	std::is_integral<T>::value && std::is_signed<T>::value
>::type>{
	template<class Archive>
	static void invoke(Archive& ar, T& v){
		using UT = typename std::make_unsigned<T>::type;
		decode(ar, *reinterpret_cast<UT*>(&v));
		v = (static_cast<UT>(v) >> 1) ^ (-(v & 0x01));
	}
};

// fixed
template<class T>
struct decoder<fixed<T>>{
	template<class Archive>
	static void invoke(Archive& ar, fixed<T>& v){
		io::read(ar.src_, reinterpret_cast<char*>(&v.value), sizeof(v.value)); // TODO: size not match
		boost::endian::little_to_native_inplace(v.value);
	}
};

// float point
template<class T>
struct decoder<T, typename std::enable_if<
	std::is_floating_point<T>::value
>::type>{
	template<class Archive>
	static void invoke(Archive& ar, T& v){
		// TODO:
	}
};

// container
template<class T>
struct decoder<std::vector<T>>{
	template<class Archive>
	static void invoke(Archive& ar, std::vector<T>& v){
		std::size_t size;
		decode(ar, size);
		v.resize(size);
		for(int i = 0; i < size; ++i)
			decode(ar, v[i]);
	}
};

}} // end namespace

namespace rrpc{

template<class T, class Archive>
inline void decode(Archive& ar, T& v){
	detail::decoder<T>::invoke(ar, v);
}

} // end namespace
