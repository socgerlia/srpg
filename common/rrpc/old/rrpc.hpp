#pragma once


#include "rrpc/idl.hpp"
// #include "rrpc/method.hpp"
#include "rrpc/session.hpp"
#include "rrpc/connection.hpp"

#include "rrpc/trivial_binary_iarchive.hpp"
#include "rrpc/trivial_binary_oarchive.hpp"

namespace rrpc{

struct TrivialBinaryArchive{
	using iarchive_type = trivial_binary_iarchive;
	using oarchive_type = trivial_binary_oarchive;
};

using binary_connection = basic_connection<TrivialBinaryArchive>;

} // end namespace rrpc

// server
// rrpc::method<void(std::string)> echo(1);
// rrpc::method<void(int)> echon(2);
// rrpc::method<size_t()> getTime(3);

RRPC_INTERFACE_BEGIN(test, 0)
RRPC_METHOD(echo, (), (const std::string&, const std::string&))
RRPC_METHOD(getTime, (size_t), ())
RRPC_INTERFACE_END()

struct test_interface
{
	RRPC_METHOD(echo, void(const std::string&, const std::string&));
}
