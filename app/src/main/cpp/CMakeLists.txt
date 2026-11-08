cmake_minimum_required(VERSION 3.22.1)

project("xposedmenu")

set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)
set(DOBBY_DEBUG OFF CACHE BOOL "" FORCE)

add_subdirectory(libraries/Dobby EXCLUDE_FROM_ALL)

find_library(log-lib log)
find_library(android-lib android)


add_library(colorplace SHARED
    colorplace/main.cpp
    colorplace/DartPatch.cpp
)

add_library(crossmath SHARED
    crossmath/main.cpp
    crossmath/DartPatch.cpp
)

add_library(klondike SHARED
    klondike/main.cpp
    klondike/DartPatch.cpp
)

add_library(left10 SHARED
    left10/main.cpp
    left10/DartPatch.cpp
)

add_library(minesweeper SHARED
    minesweeper/main.cpp
    minesweeper/DartPatch.cpp
)

add_library(multiplication SHARED
    multiplication/main.cpp
    multiplication/DartPatch.cpp
)
add_library(numbersum SHARED
    numbersum/main.cpp
    numbersum/DartPatch.cpp
)

add_library(tilematch SHARED
    tilematch/main.cpp
    tilematch/DartPatch.cpp
)

add_library(wordpuzzle SHARED
    wordpuzzle/main.cpp
    wordpuzzle/DartPatch.cpp
)

# 共通設定
foreach(target colorplace crossmath klondike left10 minesweeper multiplication numbersum tilematch wordpuzzle)

    target_include_directories(${target} PRIVATE
        libraries/Dobby/include
    )

    target_link_libraries(${target} PRIVATE
        dobby
        ${log-lib}
        ${android-lib}
    )

endforeach()