cmake_minimum_required(VERSION 3.17)
project(Example1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Example1 main.cpp)
target_link_libraries(Example1 PRIVATE vkbot)
