#pragma once

#include <cstdint>

namespace RR{

template<int Bytes> struct make_integer_type;
template<> struct make_integer_type<1>{ using type = uint8_t; };
template<> struct make_integer_type<2>{ using type = uint16_t; };
template<> struct make_integer_type<4>{ using type = uint32_t; };
template<> struct make_integer_type<8>{ using type = uint64_t; };
template<> struct make_integer_type<-1>{ using type = int8_t; };
template<> struct make_integer_type<-2>{ using type = int16_t; };
template<> struct make_integer_type<-4>{ using type = int32_t; };
template<> struct make_integer_type<-8>{ using type = int64_t; };

}
