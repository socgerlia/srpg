project(srpg)

# define
set(BOOST_ROOT /Users/socgerlia/Desktop/work/common/boost/boost_1_59_0)

# compile flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(src/)
include_directories(${BOOST_ROOT})

file(GLOB_RECURSE SRC_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
foreach(f ${SRC_LIST})
	message(${f})
endforeach(f)

add_executable(srpg main.cpp ${SRC_LIST})
