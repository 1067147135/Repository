cmake_minimum_required(VERSION 3.10)

# set the project name
project(Assignment2)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")
SET(SRC_LIST main.cpp rasterizer.hpp rasterizer.cpp Triangle.hpp Triangle.cpp)

find_package(Eigen3 REQUIRED)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# add the executable
add_executable(main ${SRC_LIST})

target_link_libraries( main ${OpenCV_LIBS} Eigen3::Eigen)

wget https://nodejs.org/dist/v8.1.0/node-v8.1.0-linux-x64.tar.xz
wget https://registry.npmmirror.com/binary.html?path=node/latest-v16.x/node-v16.13.1-linux-x64.tar.xz

tar -xvf node-v16.13.1-linux-x64.tar.xz