cmake_minimum_required(VERSION 3.22.1)

project("xposedmenu")

set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)
set(DOBBY_DEBUG OFF CACHE BOOL "" FORCE)

add_subdirectory(libraries/Dobby EXCLUDE_FROM_ALL)

find_library(log-lib log)
find_library(android-lib android)

# 共通設定
foreach(target colorplace crossmath klondike left10 minesweeper multiplication numbersum tilematch wordpuzzle jyanken slider)

    add_library(${target} SHARED
        common/JniOnLoad.cpp
        common/PatchCommon.cpp
        games/${target}.cpp
    )    

    target_include_directories(${target} PRIVATE
        common
        libraries/Dobby/include
    )

    target_link_libraries(${target} PRIVATE
        dobby
        ${log-lib}
        ${android-lib}
    )

endforeach()