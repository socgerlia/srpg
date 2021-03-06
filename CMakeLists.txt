set(BIN srpg)

# define
set(BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../common/boost)
set(CRYPTOPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cryptopp)
set(CPP_NETLIB ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp-netlib)

# compile flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS}")

project(${BIN})

include_directories(src/)
include_directories(common/)
include_directories(${BOOST_ROOT})
include_directories(${CRYPTOPP_ROOT})
include_directories(${CPP_NETLIB})

file(GLOB_RECURSE SRC_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
foreach(f ${SRC_LIST})
	message(${f})
endforeach(f)

link_directories(${BOOST_ROOT}/stage/lib)
link_directories(${CRYPTOPP_ROOT}/cryptopp)
link_directories(${CPP_NETLIB}/build/libs/network/src)
if(APPLE)
	link_directories(/usr/local/Cellar/openssl/1.0.2a-1/lib) # TODO: be variable
endif()

add_executable(${BIN} main.cpp ${SRC_LIST})

# libraries
set(STATIC_LIBRARIES boost_serialization boost_thread boost_system)
set(DYNAMIC_LIBRARIES cryptopp pthread cppnetlib-client-connections cppnetlib-uri ssl crypto)
if(APPLE OR NOT UNIX)
	set(LINK_COMMAND ${STATIC_LIBRARIES} ${DYNAMIC_LIBRARIES})
else()
	set(LINK_COMMAND "-Wl,-Bstatic" ${STATIC_LIBRARIES} "-Wl,-Bdynamic" ${DYNAMIC_LIBRARIES})
endif()

#target_link_libraries(${BIN} "-Wl,--whole-archive" boost_serialization "-Wl,--no-whole-archive" cryptopp)
target_link_libraries(${BIN} ${LINK_COMMAND})

