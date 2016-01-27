project(ConcurrencyThread CXX)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

set(HEADER settingsHeader.h)
set(SOURCE factorization.cpp source.cpp)

add_executable(ConcurrencyThread ${HEADER}  ${SOURCE})
target_link_libraries(test ${CMAKE_THREAD_LIBS_INIT} )
