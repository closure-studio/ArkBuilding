aux_source_directory(src ALBC_CORE_SRC_FILES)
add_library(albc SHARED ${ALBC_CORE_SRC_FILES})

target_link_libraries(albc PRIVATE albcexternals)
target_include_directories(albc PUBLIC include)

set_target_properties(albc PROPERTIES CXX_VISIBILITY_PRESET hidden)

target_compile_options(albc PRIVATE -Wall -Wextra -O3 -fPIC)
target_compile_options(albc PUBLIC -g)