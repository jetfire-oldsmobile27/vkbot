cmake_minimum_required(VERSION 3.17)
project(Example2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Example2 main.cpp)
target_link_libraries(Example2 PRIVATE vkbot)
