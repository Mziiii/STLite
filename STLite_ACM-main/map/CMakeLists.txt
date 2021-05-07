cmake_minimum_required(VERSION 3.17)
project(map)

set(CMAKE_CXX_STANDARD 14)

add_executable(map main.cpp
        exceptions.hpp
        map.hpp
        utility.hpp
        )