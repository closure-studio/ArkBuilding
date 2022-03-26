aux_source_directory(src ALBC_CORE_SRC_FILES)

function(add_albc_lib name type)
    add_library(${name} ${type} ${ALBC_CORE_SRC_FILES})

    target_link_libraries(${name} PRIVATE albcexternals pthread)
    target_include_directories(${name} PUBLIC include)

    set_target_properties(${name} PROPERTIES 
        CXX_VISIBILITY_PRESET hidden)

    target_compile_options(${name} 
        PRIVATE 
            -Wall 
            -Wextra 
            -Wno-unknown-pragmas 
            -O3 
            -fPIC)

    target_compile_options(${name} 
        PUBLIC 
            -g)
endfunction(add_albc_lib name type)

add_albc_lib(albc_static STATIC)
add_albc_lib(albc SHARED)