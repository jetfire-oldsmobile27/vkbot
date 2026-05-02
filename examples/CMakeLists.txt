cmake_minimum_required(VERSION 3.17)
project(VKBOT_Examples LANGUAGES CXX)

foreach(N 1 2)
    add_subdirectory(${N})
endforeach()
