cmake_minimum_required(VERSION 3.5)

project (forwarder)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC -Wno-deprecated-declarations -Wno-unused-variable")


#-------------------------------------------------------------------------------
# External Libraries
#-------------------------------------------------------------------------------
#set(BOOST_ROOT /usr/local/include/boost/)
#find_package(Boost 1.65 REQUIRED COMPONENTS system thread)

#-------------------------------------------------------------------------------
# Build Libraries
#-------------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/masa_protocol/include
                    /usr/local/lib)

file(GLOB forwarder_SRC "src/*.cpp")

set(forwarder_LIBS paho-mqttpp3 paho-mqtt3a)
add_library(forwarder SHARED ${forwarder_SRC})
target_link_libraries(forwarder ${forwarder_LIBS})


#-------------------------------------------------------------------------------
# Build executables
#-------------------------------------------------------------------------------
add_executable(forwarder-bsc demo/forwarder_bsc.cpp)
target_link_libraries(forwarder-bsc forwarder)

add_executable(consumer demo/consumer.cpp)
target_link_libraries(consumer forwarder)


