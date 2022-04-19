include(FindIconv)
include(FindThreads)
aux_source_directory(src ALBC_CORE_SRC_FILES)

function(add_albc_lib name type compiler_flags)
    add_library(${name} ${type} ${ALBC_CORE_SRC_FILES})
    target_link_libraries(${name} PRIVATE albcexternals)

    if (Iconv_FOUND)
        target_link_libraries(${name} PRIVATE Iconv::Iconv)
        target_compile_definitions(${name} PRIVATE ALBC_HAVE_ICONV)
    endif()

    if (Threads_FOUND)
        target_link_libraries(${name} PRIVATE Threads::Threads)
        target_compile_definitions(${name} PRIVATE ALBC_HAVE_THREADS)
    endif()

    if (WIN32)
        target_link_options(${name}
                PRIVATE
                -Wl,--output-def,lib${name}.def)
        target_link_options(${name}
                PRIVATE
                -Wl,-static)
        if (type STREQUAL SHARED)
            target_compile_definitions(${name} PUBLIC ALBC_BUILD_DLL)
        endif()
    endif()

    target_link_options(${name}
            PRIVATE
            -Wl,--exclude-libs=ALL # only public API
            )

    target_include_directories(${name} PUBLIC include)

    set_target_properties(${name} PROPERTIES
        CXX_VISIBILITY_PRESET hidden)

    target_compile_options(${name}
        PRIVATE
            -Wall
            -Wextra
            -Wno-unknown-pragmas
            -ffast-math
            -fomit-frame-pointer
            -fPIC
            ${compiler_flags})

    if (NOT WIN32)
        add_backward(${name})
        target_compile_definitions(${name} PRIVATE ALBC_HAVE_BACKWARD)
    endif()

endfunction(add_albc_lib name type)

add_albc_lib(albc SHARED "")
add_albc_lib(albc_static STATIC "")