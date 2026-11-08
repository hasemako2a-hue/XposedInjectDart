cmake_minimum_required(VERSION 3.22.1)

project("xposedmenu")

set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)
set(DOBBY_DEBUG OFF CACHE BOOL "" FORCE)

add_subdirectory(libraries/Dobby EXCLUDE_FROM_ALL)

find_library(log-lib log)
find_library(android-lib android)


add_library(colorplace SHARED
    games/colorplace/main.cpp
    games/colorplace/DartPatch.cpp
)

add_library(crossmath SHARED
    games/crossmath/main.cpp
    games/crossmath/DartPatch.cpp
)

add_library(jyanken SHARED
    games/jyanken/main.cpp
    games/jyanken/DartPatch.cpp
)

add_library(klondike SHARED
    games/klondike/main.cpp
    games/klondike/DartPatch.cpp
)

add_library(left10 SHARED
    games/left10/main.cpp
    games/left10/DartPatch.cpp
)

add_library(minesweeper SHARED
    games/minesweeper/main.cpp
    games/minesweeper/DartPatch.cpp
)

add_library(multiplication SHARED
    games/multiplication/main.cpp
    games/multiplication/DartPatch.cpp
)
add_library(numbersum SHARED
    games/numbersum/main.cpp
    games/numbersum/DartPatch.cpp
)

add_library(tilematch SHARED
    games/tilematch/main.cpp
    games/tilematch/DartPatch.cpp
)

add_library(wordpuzzle SHARED
    games/wordpuzzle/main.cpp
    games/wordpuzzle/DartPatch.cpp
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