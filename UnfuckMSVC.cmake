function(get_all_targets var)
    set(targets)
    get_all_targets_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
    set(${var} ${targets} PARENT_SCOPE)
endfunction()

macro(get_all_targets_recursive targets dir)
    get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirectories})
        get_all_targets_recursive(${targets} ${subdir})
    endforeach()

    get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${targets} ${current_targets})
endmacro()

macro(unfuck_msvc_runtime_compile_options target property_name)
    get_target_property(cflags ${target} ${property_name})
    if(NOT "${cflags}" STREQUAL "cflags-NOTFOUND")
        message(DEBUG "old ${property_name}: ${cflags}")
        list(FILTER cflags EXCLUDE REGEX "/M[DT](d|$<$<CONFIG:Debug>:d>)?")
        message(DEBUG "new ${property_name}: ${cflags}")
        set_target_properties(${target} PROPERTIES ${property_name} "${cflags}")
    endif()
endmacro()

macro(unfuck_msvc_runtime target)
    message(DEBUG "unfucking ${target} for MSVC")
    unfuck_msvc_runtime_compile_options(${target} COMPILE_OPTIONS)
    unfuck_msvc_runtime_compile_options(${target} INTERFACE_COMPILE_OPTIONS)
endmacro()

message(STATUS "Unfucking MSVC runtime flags for all targets")
get_all_targets(all_targets)

foreach(target ${all_targets})
    unfuck_msvc_runtime(${target})
endforeach()
