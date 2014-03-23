cmake_minimum_required(VERSION 2.8)

option(BUILD_x64 "Build for 64 bit platforms" ON)
option(BUILD_x86 "Build for 32 bit platforms" OFF)

project(shmup-game)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(game_headers
    include/game.h
    include/utils.h
    include/shaders.h
    include/meshes.h
    include/textures.h
    include/sounds.h
    include/particles.h
    include/draw.h
    include/wave.h
    include/targa.h
    include/wavefront.h
    include/stack.h
    include/resources/resources.h
)

set(game_sources
    src/game.c
    src/utils.c
    src/shaders.c
    src/meshes.c
    src/textures.c
    src/sounds.c
    src/particles.c
    src/draw.c
    src/wave.c
    src/targa.c
    src/wavefront.c
    src/stack.c
)

include_directories("./include")
include_directories("./lib/gl3")

if(BUILD_x64)
list(APPEND CMAKE_C_FLAGS "-std=c11 -Wall -w -pedantic -static-libgcc -U__STRICT_ANSI__")
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64)
endif()

if(BUILD_x86)
list(APPEND CMAKE_C_FLAGS "-m32 -std=c11 -Wall -w -pedantic -static-libgcc -U__STRICT_ANSI__")
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x86)
endif()

add_executable(shmup WIN32 ${game_headers} ${game_sources})
target_link_libraries(shmup -lnative -lgl3 -lopengl32 -lopenal32 -lmingw32)
