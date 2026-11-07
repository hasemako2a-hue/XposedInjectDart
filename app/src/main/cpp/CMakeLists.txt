
cmake_minimum_required(VERSION 3.22.1)

project("xposedmenu")

# Dobby
set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)
set(DOBBY_DEBUG OFF CACHE BOOL "" FORCE)

add_subdirectory(
    libraries/Dobby
    EXCLUDE_FROM_ALL
)

# Native library
add_library(
    ${CMAKE_PROJECT_NAME}
    SHARED
    main.cpp
    DartPatch.cpp
)

target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
    libraries/Dobby/include
)

find_library(log-lib log)

target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
    dobby
    ${log-lib}
    android
)
