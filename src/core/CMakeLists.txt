include(FindICU)
include(FindThreads)
aux_source_directory(src ALBC_CORE_SRC_FILES)

option(ALBC_ENABLE_THREADED_LOGGING "Enable threaded logging" OFF)
option(ALBC_ENABLE_BACKWARD "Enable backward" OFF)

function(add_albc_lib name type compiler_flags)
    add_library(${name} ${type} ${ALBC_CORE_SRC_FILES})
    target_link_libraries(${name} PRIVATE albcexternals)

    if (ICU_I18N_FOUND AND ICU_UC_FOUND)
        target_link_libraries(${name} PRIVATE ICU::i18n ICU::uc)
        target_compile_definitions(${name} PRIVATE ALBC_HAVE_ICU)
    endif()

	if (THREADS_FOUND)
	    target_link_libraries(${name} PRIVATE Threads::Threads)
        target_compile_definitions(${name} PRIVATE ALBC_HAVE_THREADS)
	endif()

	if (ALBC_ENABLE_THREADED_LOGGING)
	    target_compile_definitions(${name} PRIVATE ALBC_ENABLE_THREADED_LOGGING)
	endif()

    if (WIN32)
        if (type STREQUAL SHARED)
            target_compile_definitions(${name} PUBLIC ALBC_BUILD_DLL)
        endif()
    endif()

	if (MINGW)
		target_link_options(${name}
                PRIVATE
                -Wl,--output-def,lib${name}.def)

        target_link_libraries(${name} PRIVATE -static gcc stdc++ winpthread)
	endif()

    if (NOT MSVC)
	    target_link_options(${name}
            PRIVATE
            -Wl,--exclude-libs=ALL,--no-undefined
            )
	endif() 

    target_include_directories(${name} PUBLIC include)

    set_target_properties(${name} PROPERTIES
        CXX_VISIBILITY_PRESET hidden)

    if (NOT MSVC)
		target_compile_options(${name}
        PRIVATE
            -Wall
            -Wextra
            -Wno-unknown-pragmas
            -ffast-math
            -fPIC
            -pedantic
            ${compiler_flags})
	else()
	    target_compile_options(${name}
        PRIVATE
		    /W3
            /wd4068 
            /EHsc)
    endif()

    if (ALBC_ENABLE_BACKWARD)
        add_backward(${name})
        target_compile_definitions(${name} PRIVATE ALBC_HAVE_BACKWARD)
    endif()

endfunction(add_albc_lib name type)

if (MSVC)
    add_albc_lib(albc SHARED "")
    add_albc_lib(albc_static STATIC "")
else()
    add_albc_lib(albc SHARED "")
    add_albc_lib(albc_static STATIC "")
endif()