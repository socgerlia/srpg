set(BIN srpg)

# define
set(BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../common/boost)
set(CRYPTOPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cryptopp)

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

set(STATIC_LIBRARIES boost_serialization)
set(DYNAMIC_LIBRARIES cryptopp)
set(LINK_COMMAND "-Wl,-Bstatic" ${STATIC_LIBRARIES} "-Wl,-Bdynamic" ${DYNAMIC_LIBRARIES})

#target_link_libraries(${BIN} "-Wl,--whole-archive" boost_serialization "-Wl,--no-whole-archive" cryptopp)
target_link_libraries(${BIN} ${LINK_COMMAND})

