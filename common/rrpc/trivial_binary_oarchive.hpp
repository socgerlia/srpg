#pragma once

#include <boost/archive/detail/common_oarchive.hpp>
#include <boost/iostreams/write.hpp>
#include "rrpc/encode.hpp"

namespace RR{
namespace rrpc{

template<class Sink>
class trivial_binary_oarchive
	: public boost::archive::detail::common_oarchive<trivial_binary_oarchive<Sink>>
{
	Sink* snk_;

public:
	trivial_binary_oarchive(Sink* snk = nullptr) : snk_(snk){}

	const Sink* get_sink() const{ return snk_; }
	void set_sink(Sink* snk){ snk_ = snk; }

	void save(const std::string& s){
		*this & s.size();
		save_binary(s.data(), s.size());
	}
	void save(const std::wstring& s){
		*this & s.size();
		save_binary(s.data(), s.size());
	}

    void save_binary(const void* address, std::size_t count){
		boost::iostreams::write(snk_, reinterpret_cast<const char*>(address), count);
	}

protected:
	template<class T>
	typename std::enable_if<can_encode<T>>::type save_override(T& t){
		encode(snk_, t);
	}

	template<class T>
	typename std::enable_if<!can_encode<T>>::type save_override(T& t){
		boost::archive::save(*this->This(), t);
	}
};

template<class Sink>
trivial_binary_oarchive<Sink> make_trivial_binary_oarchive(Sink* snk){
	return {snk};
}

} // end namespace rrpc
} // end namespace RR
