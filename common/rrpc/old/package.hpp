#pragma once

#include <type_traits>
#include <array>
#include <tuple>

#include <boost/asio.hpp>

#include "rrpc/conversion.hpp"
#include "rrpc/streambuf.hpp"
#include "rrpc/header.hpp"

namespace rrpc{

namespace detail{
	template<class Archive>
	inline void serialize(Archive& ar){
	}
	template<class Archive, class T, class... Args>
	inline void serialize(Archive& ar, const T& t, const Args&... args){
		ar << t;
		serialize(ar, args...);
	}
}

template<class Archive>
class package{
public:
	using archive_type = Archive;
	using streambuf_type = streambuf;
	using buffers_type = std::array<boost::asio::const_buffer, 3>;

protected:
	buffers_type _buffers;

	union{
		uint32_t _packageSize;
		uint32_t _packageSizeBuf[1];
	};
	streambuf_type _packetSizesBuf;
	streambuf_type _streamBuf;

	uint32_t _lastSize;
	// std::vector<header> _headerList;

public:
	package()
		: _lastSize(0)
	{}

	streambuf_type& get_stream_buf(){
		return _streamBuf;
	}

	void pack(){
		write_packet_size(0); // packet sizes buffer ending flag

		_packageSize = _packetSizesBuf.size() + _streamBuf.size();
		native_to_little(_packageSize);

		_buffers[0] = boost::asio::buffer(_packageSizeBuf);
		_buffers[1] = boost::asio::buffer(_packetSizesBuf.data());
		_buffers[2] = boost::asio::buffer(_streamBuf.data());

		_lastSize = 0;
	}

	buffers_type& data(){
		return _buffers;
	}

	uint32_t size() const{
		return _packetSizesBuf.size() + _streamBuf.size();
	}

	void consume(){
		_packetSizesBuf.consume(_packetSizesBuf.size());
		_streamBuf.consume(_streamBuf.size());
	}

	void clear(){
		_lastSize = 0;
	}

	template<class... Args>
	void write(const Args&... args){
		archive_type oar(_streamBuf);
		detail::serialize(oar, args...);
	}

	template<class T>
	void write_packet_size(const T& v){
		archive_type oar(_packetSizesBuf);
		oar << v;
	}

	// packet
	void begin_packet(){
	}

	// template<class... Args>
	// void write(const Args&... args){
	// 	archive_type oar(get_stream_buf());
	// 	detail::serialize(oar, args...);
	// }

	void end_packet(){
		uint32_t newSize = _streamBuf.size();
		write_packet_size(newSize - _lastSize);
		_lastSize = newSize;
	}

	// template<class T>
	// T read(){
	// 	T ret;
	// 	iarchive_type iar(get_stream_buf());
	// 	iar >> ret;
	// 	return ret;
	// }
};

namespace detail{
template<class T>
struct to_deserialize_type : std::remove_cv<
	typename std::remove_reference<T>::type
>{};

template<class... Args, class IArchive>
std::tuple<typename to_deserialize_type<Args>::type...> read_args(IArchive& iar){
	std::tuple<typename to_deserialize_type<Args>::type...> ret;
	iar >> ret;
	return ret;
};
}

template<class Archive>
class ipackage{
public:
	using archive_type = Archive;
	using streambuf_type = streambuf;

protected:
	union{
		uint32_t _packageSize;
		uint32_t _packageSizeBuf[1];
	};
	std::vector<uint32_t> _packetSizes;
	streambuf_type _streamBuf;

public:
	ipackage(){
	}

	boost::asio::mutable_buffers_1 get_package_size_buf(){
		return boost::asio::buffer(_packageSizeBuf);
	}

	streambuf_type& get_stream_buf(){
		return _streamBuf;
	}

	void unpack_package_size(){
		little_to_native(_packageSize);
	}

	uint32_t size() const{
		return _packageSize;
	}

	// void pack(){
	// 	write_packet_size(0); // packet sizes buffer ending flag

	// 	_packageSize = _packetSizesBuf.size() + _streamBuf.size();
	// 	native_to_little(_packageSize);

	// 	_buffers[0] = boost::asio::buffer(_packageSizeBuf);
	// 	_buffers[1] = boost::asio::buffer(_packetSizesBuf.data());
	// 	_buffers[2] = boost::asio::buffer(_streamBuf.data());
	// }

	// buffers_type& data(){
	// 	return _buffers;
	// }


	// void consume(){
	// 	_packetSizesBuf.consume(_packetSizesBuf.size());
	// 	_streamBuf.consume(_streamBuf.size());
	// }
	boost::asio::mutable_buffers_1 prepare(){
		return _streamBuf.prepare(_packageSize);
	}

	void commit(){
		_streamBuf.commit(_packageSize);
	}

	std::vector<uint32_t>& unpack_packet_sizes(){
		_packetSizes.clear();

		archive_type iar(_streamBuf);
		uint32_t packetSize;
		for(;;){
			iar >> packetSize;
			if(packetSize == 0)
				break;
			_packetSizes.push_back(packetSize);
		}
		return _packetSizes;
	}

	int in_avail(){
		return _streamBuf.in_avail();
	}

	void ignore(uint32_t v){
	}

	void clear(){
	}

	template<class T>
	T read(){
		T ret;
		archive_type iar(_streamBuf);
		iar >> ret;
		return ret;
	}

	void read_header(header& h){
		archive_type iar(_streamBuf);
		iar >> h;
	}

	template<class... Args>
	auto read_args()
		-> decltype(detail::read_args<Args...>(std::declval<archive_type&>()))
	{
		archive_type iar(_streamBuf);
		return detail::read_args<Args...>(iar);
	}

	// template<class T>
	// void write_packet_size(const T& v){
	// 	archive_type oar(_packetSizesBuf);
	// 	oar << v;
	// }

	// packet
	// void begin_packet(){
	// }

	// template<class... Args>
	// void write(const Args&... args){
	// 	archive_type oar(get_stream_buf());
	// 	detail::serialize(oar, args...);
	// }

	// void end_packet(){
	// 	uint32_t newSize = _streamBuf.size();
	// 	write_packet_size(newSize - _lastSize);
	// 	_lastSize = newSize;
	// }

	// template<class T>
	// T read(){
	// 	T ret;
	// 	iarchive_type iar(get_stream_buf());
	// 	iar >> ret;
	// 	return ret;
	// }
};


}
