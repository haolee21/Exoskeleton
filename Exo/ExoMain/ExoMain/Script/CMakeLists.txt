cmake_minimum_required(VERSION 3.0.0)
project(ExoController)
set(CMAKE_SYSTEM_NAME Linux) 
set(CMAKE_SYSTEM_PROCESSOR arm)

#setup boost library
set(Boost_INCLUDE_DIR /home/lowlimb/rpi-kernel/boost_1_71_0)
set(Boost_LIBRARY_DIR /home/lowlimb/rpi-kernel/boost_1_71_0/stage/lib)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(Boost_USE_STATIC_LIBS ON)
# set(XENO_CONFIG "/home/lowlimb/rpi-kernel/xenomai-3.0.7/target/usr/xenomai/bin/xeno-config")
# set(XENO_CFLAG_PARAM  "--alchemy --cflags")
# set(XENO_LDFLAG_PARAM  "--alchemy --ldflags")
# execute_process(COMMAND bash -c "${XENO_CONFIG} ${XENO_CFLAG_PARAM}" 
#                 OUTPUT_VARIABLE XENO_CFFLAGS
#                 OUTPUT_STRIP_TRAILING_WHITESPACE)
# execute_process(COMMAND bash -c "${XENO_CONFIG} ${XENO_LDFLAG_PARAM}"
#                 OUTPUT_VARIABLE XENO_LDFLAG
#                 OUTPUT_STRIP_TRAILING_WHITESPACE)

# we can only set flags after we called xeno-config since xeno requires many flags when using make


# we can only set flags after we called xeno-config since xeno requires many flags when using make

# set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -o3 -pthread ${XENO_CFFLAGS} ${XENO_LDFLAG}" ) #you can set with add_definitions("-Wall -std=c+11, etc")
# set(CMAKE_C_FLAGS   "-std=c++17 -Wall -o3 -pthread ${XENO_CFFLAGS} ${XENO_LDFLAG}")
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -o3 -pthread -lrt -o3 ") #you can set with add_definitions("-Wall -std=c+11, etc")
set(CMAKE_C_FLAGS   "-std=c++17 -Wall -o3 -pthread -lrt -o3 ")

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
    #${Eigen_INCLUDE_DIR}
    #${Xenomai_INCLUDE_DIR}
    
)
file(GLOB HEAD_FILES ${Include_DIR}/*.h ${Include_DIR}/*.hpp})
add_library(
    lib STATIC
    ${HEAD_FILES}
    


)
set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX)
link_directories(${Boost_LIBRARY_DIR}) 
enable_testing()
file(GLOB SRC_FILES ${Include_DIR}/*.cpp)
add_executable(exo_o src/main.cpp ${SRC_FILES})

target_link_libraries(exo_o PRIVATE ${Boost_LIBRARIES}) 

