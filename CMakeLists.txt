set(BIN srpg)

# define
set(BOOST_ROOT /Users/socgerlia/Desktop/work/common/boost)
set(CRYPTOPP_ROOT /Users/socgerlia/Desktop/work/common/cryptopp)

# compile flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS}")

project(${BIN})

include_directories(src/)
include_directories(common/)
include_directories(${BOOST_ROOT})
include_directories(${CRYPTOPP_ROOT})

file(GLOB_RECURSE SRC_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
foreach(f ${SRC_LIST})
	message(${f})
endforeach(f)

link_directories(${BOOST_ROOT}/stage/lib)
link_directories(${CRYPTOPP_ROOT}/cryptopp)

add_executable(${BIN} main.cpp ${SRC_LIST})

target_link_libraries(${BIN} boost_serialization cryptopp)
