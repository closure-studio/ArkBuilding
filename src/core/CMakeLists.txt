aux_source_directory(src ALBC_CORE_SRC_FILES)

function(add_albc_lib name type)
    add_library(${name} ${type} ${ALBC_CORE_SRC_FILES})

    target_link_libraries(${name} PRIVATE albcexternals pthread)
    if (WIN32)
        target_link_options(${name} PRIVATE -Wl,--output-def,lib${name}.def)
    endif()

    target_link_options(${name} PRIVATE -Wl,--exclude-libs=ALL) # only public API

    target_include_directories(${name} PUBLIC include)

    set_target_properties(${name} PROPERTIES
        CXX_VISIBILITY_PRESET hidden)

    target_compile_options(${name}
        PRIVATE
            -static
            -Wall
            -Wextra
            -Wno-unknown-pragmas
            -ffast-math
            -fomit-frame-pointer
            -O3
            -fPIC)

endfunction(add_albc_lib name type)

add_albc_lib(albc SHARED)
add_albc_lib(albc_static STATIC)