project(albcexample)

aux_source_directory(src ALBC_EXAMPLE_SRC)
add_executable(albcexample ${ALBC_EXAMPLE_SRC})
target_link_libraries(albcexample albc)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../test DESTINATION ${CMAKE_BINARY_DIR})