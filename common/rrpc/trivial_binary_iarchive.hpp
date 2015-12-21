#pragma once

#include <boost/mpl/bool.hpp>
#include "decode.hpp"

namespace rrpc{

template<class Source>
class trivial_binary_iarchive{
	template<class T, class Enable> friend struct detail::decoder;

	Source& src_;

public:
	trivial_binary_iarchive(Source& src)
		: src_(src)
	{
	}

    // Implement requirements for archive concept
    typedef boost::mpl::bool_<true> is_loading;
    typedef boost::mpl::bool_<false> is_saving;
    template<class T> void register_type(){}

	template<class T>
	trivial_binary_iarchive& operator>>(T& v){
		decode(*this, v);
		return *this;
	}
	template<class T>
	trivial_binary_iarchive& operator&(T& v){
		return *this >> v;
	}
};

template<class Source>
trivial_binary_iarchive<Source> make_trivial_binary_iarchive(Source& src){
	return {src};
}

} // end namespace rrpc
