cmake_minimum_required(VERSION 3.22.1)

project("xposedmenu")

set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)
set(DOBBY_DEBUG OFF CACHE BOOL "" FORCE)

add_subdirectory(libraries/Dobby EXCLUDE_FROM_ALL)

find_library(log-lib log)
find_library(android-lib android)


add_library(wordpuzzle SHARED
    colorplace/main.cpp
    colorplace/DartPatch.cpp
)

add_library(examplegame SHARED
    crossmath/main.cpp
    crossmath/GamePatch.cpp
)

add_library(otherapp SHARED
    klondike/main.cpp
    klondike/OtherPatch.cpp
)

add_library(wordpuzzle SHARED
    left10/main.cpp
    left10/DartPatch.cpp
)

add_library(examplegame SHARED
    minesweeper/main.cpp
    minesweeper/GamePatch.cpp
)

add_library(otherapp SHARED
    multiplication/main.cpp
    multiplication/OtherPatch.cpp
)
add_library(wordpuzzle SHARED
    numbersum/main.cpp
    numbersum/DartPatch.cpp
)

add_library(examplegame SHARED
    tilematch/main.cpp
    tilematch/GamePatch.cpp
)

add_library(otherapp SHARED
    wordpuzzle/main.cpp
    wordpuzzle/OtherPatch.cpp
)

# 共通設定
foreach(target wordpuzzle examplegame otherapp)

    target_include_directories(${target} PRIVATE
        libraries/Dobby/include
    )

    target_link_libraries(${target} PRIVATE
        dobby
        ${log-lib}
        ${android-lib}
    )

endforeach()