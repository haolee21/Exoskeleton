cmake_minimum_required(VERSION 3.0.0)
project(TestProject)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pthread -lboost_serialization -lwiringPi") #you can set with add_definitions("-Wall -std=c+11, etc")
set(Boost_INCLUDE_DIR /home/pi/Lib/boost_1_70_0)
set(Boost_LIBRARY_DIR /home/pi/Lib/boost_1_70_0/stage/lib)
find_package(Boost COMPONENTS serialization date_time filesystem REQUIRED)

include_directories(
    include
    src
    ${Boost_INCLUDE_DIR}
)
add_library(
    lib STATIC
    include/Recorder.hpp 
    include/RecData.hpp
    include/PWM.h
    include/Valve.h
    include/Controller.h
    include/Sensor.h

)
set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX)
link_directories(${Boost_LIBRARY_DIR})

enable_testing()
add_executable(exo_o src/main.cpp include/Controller.cpp include/PWM.cpp include/Recorder.cpp include/Sensor.cpp include/Valve.cpp)

target_link_libraries(exo_o PRIVATE ${Boost_LIBRARIES})

