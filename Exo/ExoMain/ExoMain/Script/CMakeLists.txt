cmake_minimum_required(VERSION 3.0.0)
project(TestProject)
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -pthread -lwiringPi -o3") #you can set with add_definitions("-Wall -std=c+11, etc")
set(Boost_INCLUDE_DIR /home/pi/Lib/boost_1_70_0)
set(Boost_LIBRARY_DIR /home/pi/Lib/boost_1_70_0/stage/lib)
set(Eigen_INCLUDE_DIR /home/pi/Lib/eigen-eigen-323c052e1731)
find_package(Boost COMPONENTS serialization date_time filesystem REQUIRED)

include_directories(
    include
    src
    ${Boost_INCLUDE_DIR}
    ${Eigen_INCLUDE_DIR}
)
add_library(
    lib STATIC
    include/Recorder.hpp 
    include/RecData.hpp
    include/PWM.h
    include/Valve.h
    include/Controller.h
    include/Sensor.h
    include/Displayer.hpp

)
set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX)
link_directories(${Boost_LIBRARY_DIR})

enable_testing()
add_executable(exo_o src/main.cpp include/Controller.cpp include/PWM.cpp include/Sensor.cpp include/Valve.cpp include/Displayer.cpp)

target_link_libraries(exo_o PRIVATE ${Boost_LIBRARIES})

