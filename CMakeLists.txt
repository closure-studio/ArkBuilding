cmake_minimum_required (VERSION 3.8)
project(albc)

set(CMAKE_CXX_STANDARD 17)
set(LLVM_ENABLE_RTTI ON)
set(LLVM_ENABLE_EH ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC -funroll-loops -ffast-math -pthread")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -O2 -g")
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ ${CMAKE_CXX_STANDARD_LIBRARIES}")

configure_file("test/player_data.json" "player_data.json" COPYONLY)
configure_file("test/building_data.json" "building_data.json" COPYONLY)

include_directories(exec)
include_directories(include)
include_directories(include/json)
include_directories(include/alglib)

add_executable(albc
        exec/main.cpp
        algorithm.cpp
        building_data_model.cpp
        jsoncpp.cpp
        player_building_model.cpp
        player_data_model.cpp
        worker.cpp
        go_bridge.cpp
        go_bridge.h
        locale_util.cpp
        include/operator_pattern.h)

