# CMakeLists.txt
cmake_minimum_required(VERSION 3.0)
project(test)

find_package(Protobuf CONFIG NAMES protobuf)

add_executable(main main.cpp)
target_link_libraries(main protobuf)
