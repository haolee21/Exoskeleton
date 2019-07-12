cmake_minimum_required(VERSION 3.0.0)
project(TestProject)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pthread -lboost_serialization") #you can set with add_definitions("-Wall -std=c+11, etc")
set(Boost_INCLUDE_DIR /home/lowlimb/Downloads/boost_1_70_0)
set(Boost_LIBRARY_DIR /home/lowlimb/Downloads/boost_1_70_0/stage/lib)
find_package(Boost COMPONENTS system filesystem serialization REQUIRED)

include_directories(
    include
    src
    ${Boost_INCLUDE_DIR}
)
add_library(
    lib1 STATIC
    include/lib1.hpp
    include/lib1.cpp

)
link_directories(${Boost_LIBRARY_DIR})

enable_testing()
add_executable(hello_o src/main.cpp)

target_link_libraries(hello_o PRIVATE lib1 ${Boost_LIBRARIES})

