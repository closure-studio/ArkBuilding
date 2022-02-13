cmake_minimum_required(VERSION 3.18)
project(albc)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXXFLAGS "-Wall -Wextra -std=c++17 -fPIC -funroll-loops -ffast-math -static -static-libgcc -static-libstdc++")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -DNDEBUG")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO "$ENV{CXXFLAGS} -O2 -g")
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ ${CMAKE_CXX_STANDARD_LIBRARIES}")

include_directories(exec)
include_directories(include)
include_directories(include/json)

add_executable(albc
        exec/main.cpp
        algorithm.cpp
        building_data_model.cpp
        jsoncpp.cpp
        player_building_model.cpp
        player_data_model.cpp
        worker.cpp
        go_bridge.cpp
        go_bridge.h)
