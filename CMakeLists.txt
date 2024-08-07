cmake_minimum_required(VERSION 3.22.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(SortSim C)

add_executable(${PROJECT_NAME} 
    src/main.c
    src/visualizer.c
    src/visualizer.h
    src/sorts/sorts.c
    src/sorts/sorts.h
    src/sorts/bubble_sort.c
    src/sorts/bubble_sort.h
    src/sorts/selection_sort.c
    src/sorts/selection_sort.h
    src/sorts/insertion_sort.c
    src/sorts/insertion_sort.h
    src/sorts/shell_sort.c
    src/sorts/shell_sort.h
    src/sorts/cocktail_shaker_sort.c
    src/sorts/cocktail_shaker_sort.h
    src/sorts/merge_sort.c
    src/sorts/merge_sort.h
    src/sorts/quick_sort.c
    src/sorts/quick_sort.h
    src/sorts/bogo_sort.c
    src/sorts/bogo_sort.h
    src/sorts/heap_sort.h
    src/sorts/heap_sort.c
)

add_subdirectory(raylib)

target_include_directories(${PROJECT_NAME} 
    SYSTEM PRIVATE raylib/include
    PUBLIC include
    PUBLIC src
)

target_link_directories(${PROJECT_NAME}
    PRIVATE raylib/src
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC raylib
)

if(MSVC)
    set_property(TARGET ${PROJECT_NAME}  PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4996 /external:W0 /experimental:c11atomics)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow -pedantic -Wcast-align -Wunused -Wpedantic -Wconversion -Wsign-conversion)
endif()