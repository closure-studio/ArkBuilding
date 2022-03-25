project(albcexternals)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

add_subdirectory(concurrentqueue)
add_subdirectory(coin)

set(COIN_LIBS_PATH
        coin/Cbc/CoinUtils
        coin/Cbc/Clp
        coin/Cbc/Clp/src/OsiClp
        coin/Cbc/Osi/src/Osi
        coin/Cbc/Cbc
        coin/Cbc/Cbc/src/OsiCbc)

foreach(name AllDifferent Clique DuplicateRow FlowCover GMI Gomory
              KnapsackCover LandP LiftAndProject MixedIntegerRounding
              MixedIntegerRounding2 OddHole PreProcess Probing RedSplit
              RedSplit2 ResidualCapacity SimpleRounding Twomir ZeroHalf)
    set(COIN_LIBS_PATH ${COIN_LIBS_PATH} coin/Cbc/Cgl/src/Cgl${name})
endforeach()

set(COIN_INCLUDE_DIRS
        coin/Cbc/BuildTools/headers)

function(add_coin_library path)
    get_filename_component(name ${path} NAME)
    set(COIN_LIBS ${COIN_LIBS} ${name} PARENT_SCOPE)

    set(src_path ${path})
    if (EXISTS ${PROJECT_SOURCE_DIR}/${path}/src)
        set(src_path ${path}/src)
    endif ()

    set(COIN_INCLUDE_DIRS ${COIN_INCLUDE_DIRS} ${src_path} PARENT_SCOPE)
    message(STATUS "Added ${name} at ${path}")
endfunction()

foreach(path ${COIN_LIBS_PATH})
    add_coin_library(${path})
endforeach()

message(STATUS "Coin libraries: ${COIN_LIBS}")
message(STATUS "Coin include dirs: ${COIN_INCLUDE_DIRS}")

set(ALBC_EXTERNAL_LIBS
        ${COIN_LIBS}
        concurrentqueue
        jsoncpp_lib)

set(ALBC_EXTERNAL_INCLUDE_DIRS
        ${COIN_INCLUDE_DIRS}
        concurrentqueue
        jsoncpp/include)

aux_source_directory(. ALBC_EXTERNAL_SRC_FILES)

add_library(albcexternals STATIC ${ALBC_EXTERNAL_SRC_FILES})
find_package(jsoncpp CONFIG REQUIRED)
target_link_libraries(albcexternals PUBLIC ${ALBC_EXTERNAL_LIBS})
target_include_directories(albcexternals PUBLIC ${ALBC_EXTERNAL_INCLUDE_DIRS} include)
target_compile_definitions(albcexternals PUBLIC HAVE_CONFIG_H=1)