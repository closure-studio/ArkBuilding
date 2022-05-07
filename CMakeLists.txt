cmake_minimum_required (VERSION 3.15)
cmake_policy(SET CMP0091 NEW)  # MSVC runtime library flags are selected by an abstraction
project(albc)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

set(LLVM_ENABLE_RTTI ON)
set(LLVM_ENABLE_EH ON)

add_subdirectory(src)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test DESTINATION ${CMAKE_BINARY_DIR}/out)

# remove manually added MSVC runtime library flags
if(MSVC)
    include(UnfuckMSVC.cmake)
endif()
