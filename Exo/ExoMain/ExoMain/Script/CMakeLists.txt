cmake_minimum_required(VERSION 3.0.0)
project(ExoController)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pthread -o3") #you can set with add_definitions("-Wall -std=c+11, etc")
set(Boost_INCLUDE_DIR /home/lowlimb/rpi-kernel/boost_1_71_0)
set(Boost_LIBRARY_DIR /home/lowlimb/rpi-kernel/boost_1_71_0/stage/lib)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# Search for programs only in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers only in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(Include_DIR include)
find_package(Boost 1.71.0 COMPONENTS serialization date_time filesystem REQUIRED)
#set(Eigen_INCLUDE_DIR /home/pi/Lib/eigen-eigen-323c052e1731)







include_directories(
    ${Include_DIR}
    src
    ${Boost_INCLUDE_DIR}
    ${Eigen_INCLUDE_DIR}
    
)
add_library(
    lib STATIC
    ${Include_DIR}/common.hpp
    ${Include_DIR}/Recorder.hpp 
    ${Include_DIR}/RecData.hpp
    ${Include_DIR}/Pin.h
    ${Include_DIR}/PWM.h
    ${Include_DIR}/Valve.h
    ${Include_DIR}/PIDCon.h
    ${Include_DIR}/FSMachine.hpp
    ${Include_DIR}/Controller.h
    ${Include_DIR}/BWFilter.hpp
    ${Include_DIR}/Sensor.h
    ${Include_DIR}/Displayer.hpp


)
set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX)
link_directories(${Boost_LIBRARY_DIR})

enable_testing()
add_executable(exo_o src/main.cpp ${Include_DIR}/Pin.cpp ${Include_DIR}/BWFilter.cpp ${Include_DIR}/Controller.cpp ${Include_DIR}/PWM.cpp ${Include_DIR}/Sensor.cpp ${Include_DIR}/Valve.cpp ${Include_DIR}/Displayer.cpp ${Include_DIR}/PIDCon.cpp ${Include_DIR}/FSMachine.cpp)

target_link_libraries(exo_o PRIVATE ${Boost_LIBRARIES} )

