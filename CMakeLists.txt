cmake_minimum_required (VERSION 3.13)
project(albc)

set(LLVM_ENABLE_RTTI ON)
set(LLVM_ENABLE_EH ON)

configure_file("test/player_data.json" "test/player_data.json" COPYONLY)
configure_file("test/building_data.json" "test/building_data.json" COPYONLY)

add_subdirectory(src)