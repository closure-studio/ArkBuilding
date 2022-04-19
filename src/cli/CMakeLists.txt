aux_source_directory(src ALBC_CLI_SRC_FILES)

add_executable(albccli ${ALBC_CLI_SRC_FILES})
target_link_libraries(albccli albc)
target_compile_options(albccli PRIVATE -g)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../test DESTINATION ${CMAKE_BINARY_DIR})