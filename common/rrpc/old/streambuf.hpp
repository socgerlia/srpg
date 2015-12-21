#pragma once

#include <boost/asio.hpp>

namespace rrpc{

class streambuf : public boost::asio::streambuf{
public:
	using std::streambuf::gbump;
};

/*
#include <stdexcept>
#include <streambuf>
#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/throw_exception.hpp>

template<size_t MaxSize=1024, class CharT=char>
class one_time_ostreambuf : public std::basic_streambuf<CharT>{
protected:
	char_type[MaxSize] _buffer;
	char_type* _last_delta_ptr;

	virtual int_type overflow(int_type c) override{
        std::length_error ex("[rrpc::one_time_ostreambuf] buffer is too small");
        boost::asio::detail::throw_exception(ex);

		return traits_type::not_eof(c);
	}

public:
	typedef boost::asio::const_buffers_1 const_buffers_type;
	typedef boost::asio::mutable_buffers_1 mutable_buffers_type;

	one_time_ostreambuf() : _last_delta_ptr(_buffer){
		clear();
	}

	std::size_t size() const{
		return pptr() - _buffer;
	}
	std::size_t delta_size() const{
		return pptr() - _last_delta_ptr;
	}
	std::size_t max_size() const{
		return MaxSize;
	}

	const_buffers_type data() const{
		return boost::asio::buffer(boost::asio::const_buffer(
			_buffer, size()*sizeof(char_type)
		));
	}
	const_buffers_type delta_data(){
		char_type* begin = _last_delta_ptr;
		_last_delta_ptr = pptr();

		return boost::asio::buffer(boost::asio::const_buffer(
			begin, (pptr() - begin)*sizeof(char_type)
		));
	}

	void empty(){
		return pptr() == _buffer;
	}
	void clear(){
		setp(_buffer, _buffer+MaxSize);
		_last_delta_ptr = _buffer;
	}
};
*/

} // end namespace rrpc
