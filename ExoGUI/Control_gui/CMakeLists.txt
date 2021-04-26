cmake_minimum_required(VERSION 3.5)

project(Exo_PC_gui LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_INCLUDE_DIR /home/lowlimb/tools/x86/boost_1_76_0)
set(Boost_LIBRARY_DIR /home/lowlimb/tools/x86/boost_1_76_0/stage/lib)
set(Boost_USE_STATIC_LIBS ON)
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -o3 -pthread -lrt ") #you can set with add_definitions("-Wall -std=c+11, etc")
set(CMAKE_C_FLAGS   "-std=c++17 -Wall -o3 -pthread -lrt ")


# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)
find_package(Boost 1.76.0 COMPONENTS serialization)
include_directories(
  src
  include
  ${Boost_INCLUDE_DIR}
)
file(GLOB HEAD_FILES include/*.h })
message(STATUS "Header")
message(STATUS ${HEAD_FILES})
 add_library(lib static
  ${HEAD_FILES}
  
)
set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX)
file(GLOB SRC_FILES src/*.cpp)
message(STATUS "Source")
message(STATUS ${SRC_FILES})

# link_directories(${Boost_LIBRARY_DIR}) 
add_executable(Exo_PC_gui 
  ${SRC_FILES}
  ${HEAD_FILES}
  ${Boost_INCLUDE_DIR}
  
)




target_link_libraries(Exo_PC_gui PRIVATE 
  Qt5::Widgets 
  Qt5::Charts
  Qt5::Core 
  Qt5::Gui
  ${Boost_LIBRARY_DIR})

