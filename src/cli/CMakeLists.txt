aux_source_directory(src ALBC_CLI_SRC_FILES)

add_executable(albccli ${ALBC_CLI_SRC_FILES})
target_link_libraries(albccli albc)