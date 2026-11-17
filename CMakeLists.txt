cmake_minimum_required(VERSION 3.24)

project(PermissionDeniedNative
    VERSION 0.1.0
    DESCRIPTION "Native C++ engine for Permission Denied"
    LANGUAGES C CXX
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
# SDL2 is maintenance-only; pin SDL3 to a stable release.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.20
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL3)

add_library(pd_core STATIC core/src/game.cpp)
target_include_directories(pd_core PUBLIC core/include)
target_compile_features(pd_core PUBLIC cxx_std_20)

add_library(pd_renderer STATIC
    renderer/src/gl_api.cpp
    renderer/src/opengl_renderer.cpp
)
target_include_directories(pd_renderer PUBLIC renderer/include core/include PRIVATE renderer/src)
target_link_libraries(pd_renderer PUBLIC SDL3::SDL3)
target_compile_definitions(pd_renderer PRIVATE PD_OPENGL_ES=0)

if(WIN32)
    target_link_libraries(pd_renderer PRIVATE opengl32)
endif()

add_executable(pd_desktop
    desktop/main.cpp
    platform/sdl/sdl_application.cpp
)
target_include_directories(pd_desktop PRIVATE platform/sdl)
target_link_libraries(pd_desktop PRIVATE pd_core pd_renderer SDL3::SDL3)
if(WIN32)
    target_link_libraries(pd_desktop PRIVATE opengl32)
endif()

if(MSVC)
    foreach(target pd_core pd_renderer pd_desktop)
        target_compile_options(${target} PRIVATE /W4 /permissive- /Zc:__cplusplus)
    endforeach()
else()
    foreach(target pd_core pd_renderer pd_desktop)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()

set_target_properties(pd_desktop PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
include(CTest)
if(BUILD_TESTING)
    add_executable(pd_core_smoke core/tests/core_smoke.cpp)
    target_link_libraries(pd_core_smoke PRIVATE pd_core)
    add_test(NAME pd_core_smoke COMMAND pd_core_smoke)
endif()

message(STATUS "Permission Denied native target: desktop")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "SDL3 source: ${sdl3_SOURCE_DIR}")
