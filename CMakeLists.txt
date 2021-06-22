cmake_minimum_required(VERSION 3.5)

project (forwarder)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgdal -pthread -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC -Wno-deprecated-declarations -Wno-unused-variable")


#-------------------------------------------------------------------------------
# External Libraries
#-------------------------------------------------------------------------------
#set(BOOST_ROOT /usr/local/include/boost/)
#find_package(Boost 1.65 REQUIRED COMPONENTS system thread)
find_package(Eigen3 REQUIRED)

find_package(Python3 COMPONENTS Interpreter Development)
message("Python_FOUND:${Python3_FOUND}")
message("Python_VERSION:${Python3_VERSION}")
message("Python_Development_FOUND:${Python3_Development_FOUND}")
message("Python_LIBRARIES:${Python3_LIBRARIES}")
message("Python_INCLUDE_DIRS:${Python3_INCLUDE_DIRS}")

#-------------------------------------------------------------------------------
# Build Libraries
#-------------------------------------------------------------------------------
add_subdirectory(class-tracker)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/masa_protocol/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/class-tracker/matplotlib-cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/class-tracker/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/class-tracker/geodetic_utils/geodetic_utils/include/geodetic_utils 
                    ${EIGEN3_INCLUDE_DIR}
                    ${Python3_INCLUDE_DIRS}
                    /usr/include/gdal
                    /usr/local/lib)

file(GLOB forwarder_SRC "src/*.cpp")

set(forwarder_LIBS class-tracker gdal) #paho-mqttpp3 paho-mqtt3a)
add_library(forwarder SHARED ${forwarder_SRC})
target_link_libraries(forwarder ${forwarder_LIBS})


#-------------------------------------------------------------------------------
# Build executables
#-------------------------------------------------------------------------------
add_executable(forwarder-bsc demo/forwarder_bsc.cpp)
target_link_libraries(forwarder-bsc forwarder)

add_executable(consumer demo/consumer.cpp)
target_link_libraries(consumer forwarder)


