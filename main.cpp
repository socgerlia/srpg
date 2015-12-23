#include <iostream>
#include "logic/game_table.h"

#include <boost/type_index.hpp>

#define DM(x) std::cout << x << std::endl;
#define TYPENAME(t) boost::typeindex::type_id_with_cvr<t>().pretty_name()
using namespace srpg;

#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>
#include <cryptopp/aes.h>
#include <cryptopp/tea.h>

void test0(){
	using namespace std;
	using namespace CryptoPP;
	using Algorirhm = TEA;

	AutoSeededRandomPool prng;

	SecByteBlock key(Algorirhm::DEFAULT_KEYLENGTH);
	prng.GenerateBlock( key, key.size() );

	byte iv[ Algorirhm::BLOCKSIZE ];
	prng.GenerateBlock( iv, sizeof(iv) );

	string plain = "CBC Mode Test";
	string cipher, encoded, recovered;

	/*********************************\
	\*********************************/

	try
	{
	    cout << "plain text: " << plain << endl;

	    CBC_Mode< Algorirhm >::Encryption e;
	    e.SetKeyWithIV( key, key.size(), iv );

	    // The StreamTransformationFilter adds padding
	    //  as required. ECB and CBC Mode must be padded
	    //  to the block size of the cipher.
	    StringSource ss( plain, true, 
	        new StreamTransformationFilter( e,
	            new StringSink( cipher )
	        ) // StreamTransformationFilter      
	    ); // StringSource
	}
	catch( const CryptoPP::Exception& e )
	{
	    cerr << e.what() << endl;
	    exit(1);
	}

	/*********************************\
	\*********************************/

	// Pretty print cipher text
	StringSource ss( cipher, true,
	    new HexEncoder(
	        new StringSink( encoded )
	    ) // HexEncoder
	); // StringSource
	cout << "cipher text: " << encoded << endl;

	/*********************************\
	\*********************************/

	try
	{
	    CBC_Mode< Algorirhm >::Decryption d;
	    d.SetKeyWithIV( key, key.size(), iv );

	    // The StreamTransformationFilter removes
	    //  padding as required.
	    StringSource ss( cipher, true, 
	        new StreamTransformationFilter( d,
	            new StringSink( recovered )
	        ) // StreamTransformationFilter
	    ); // StringSource

	    cout << "recovered text: " << recovered << endl;
	}
	catch( const CryptoPP::Exception& e )
	{
	    cerr << e.what() << endl;
	    exit(1);
	}
}


// test for serialization
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "game_map/cmd.h"

#include "rrpc/container_device.hpp"
#include "rrpc/trivial_binary_oarchive.hpp"

#include <sstream>

namespace boost {                                    \
namespace serialization {                            \
template <class T>                                          \
struct implementation_level_impl<const std::vector<T>>                  \
{                                                    \
typedef mpl::integral_c_tag tag;                 \
typedef mpl::int_< object_serializable > type;                     \
BOOST_STATIC_CONSTANT(                           \
    int,                                         \
    value = implementation_level_impl::type::value   \
);                                               \
};                                                   \
}                                                    \
}

void test1(){
	using namespace boost::archive;
	using namespace boost::serialization;
	using namespace RR::rrpc;

	std::string str;
	{
		std::vector<cmd*> v{ new cmd_add_vertex{512, 999}, new cmd_add_group{666}, new cmd_add_vertex{128, 333} };

		std::stringstream ss;
		xml_oarchive oar(ss, boost::archive::no_header);

		// auto snk = make_container_device(str);
		// trivial_binary_oarchive<decltype(snk)> oar{&snk};

		oar.register_type<cmd>();
		oar.register_type<cmd_add_vertex>();
		oar.register_type<cmd_add_group>();
		void_cast_register<cmd_add_group, cmd>();
		void_cast_register<cmd_add_vertex, cmd>();
		oar << make_nvp("vector", v);

		str = ss.str();
	}
	DM("size = " << str.size())
	DM(str)
}

#include <boost/network/protocol/http/client.hpp>

int test2(int argc, char** argv){
	using namespace boost::network;

	if (argc != 2){
		std::cout << "Usage: " << argv[0] << " [url]" << std::endl;
		return 1;
	}

	http::client client;
	http::client::request request(argv[1]);
	request << header("Connection", "close");
	http::client::response response = client.get(request);
	std::cout << body(response) << std::endl;

	return 0;
}

int main(int argc, char** argv){
	game_table gt;

	// test0();
	test2(argc, argv);
	return 0;
}

