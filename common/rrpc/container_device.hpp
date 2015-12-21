#pragma once

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>

namespace RR{
namespace rrpc{

template<class Container>
class container_device{
protected:
	Container& container_;
	std::streampos pos_;

public:
	typedef typename Container::value_type char_type;
    typedef boost::iostreams::seekable_device_tag category;

	container_device(Container& cnt) : container_(cnt), pos_(0) { }

	std::streamsize read(char_type* s, std::streamsize n) 
	{
        std::streamsize result = std::min(n, static_cast<std::streamsize>(container_.size() - pos_));
        if(result != 0){
            std::copy(container_.begin() + pos_, container_.begin() + pos_ + result, s);
            pos_ += result;
            return result;
        }
        else{
            return -1; // EOF
        }
	}
	std::streamsize write(const char_type* s, std::streamsize n) 
	{
		std::streamsize result = 0;
		if(pos_ < container_.size()){
			std::streamsize amt = static_cast<std::streamsize>(container_.size() - pos_);
			result = std::min(n, amt);
			std::copy(s, s + result, container_.begin() + pos_);
			pos_ += result;
		}
		if (result < n){
			container_.insert(container_.end(), s, s + n);
			pos_ = container_.size();
		}
		return n;
	}
	boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way) 
	{
		boost::iostreams::stream_offset next;
		switch(way){
		case std::ios::beg: next = off; break;
		case std::ios::cur: next += off; break;
		case std::ios::end: next = container_.size() + off; break;
		default: break; // TODO: error
		}
		// if(0 < pos_ || pos_ > container_.size())
		// 	;
		return pos_ = next;
	}
};

template<class Container>
container_device<Container> make_container_device(Container& cnt){
	return {cnt};
}

} // end namespace rrpc
} // end namespace RR


