cmake_minimum_required(VERSION 2.8)

project(tile_game)

add_definitions(-D_UNICODE)

if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(/W4 /WX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
else()
    add_definitions(-std=c++11 -Wall -Werror -pedantic)
endif()

set(Boost_USE_STATIC_LIBS TRUE)
find_package(Boost 1.55 COMPONENTS atomic chrono context coroutine date_time regex system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

include_directories(
    src
    tests
    third_party
)

add_subdirectory(tests)
