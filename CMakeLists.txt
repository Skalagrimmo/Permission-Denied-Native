cmake_minimum_required(VERSION 3.24)

project(PermissionDeniedNative
    VERSION 0.1.0
    DESCRIPTION "Native C++ engine for Permission Denied"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(ANDROID)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

include(FetchContent)

# SDL Android's Java loader expects libSDL3.so alongside libmain.so.
# Keep desktop builds static to preserve the simple standalone executable.
if(ANDROID)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
else()
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
endif()
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

add_library(pd_core STATIC
    core/src/game.cpp
    core/src/movement.cpp
)
target_include_directories(pd_core PUBLIC core/include)
target_compile_features(pd_core PUBLIC cxx_std_20)

add_library(pd_renderer STATIC
    renderer/src/gl_api.cpp
    renderer/src/opengl_renderer.cpp
    renderer/src/vector_scene.cpp
)
target_include_directories(pd_renderer PUBLIC renderer/include core/include PRIVATE renderer/src)
target_link_libraries(pd_renderer PUBLIC SDL3::SDL3)

if(ANDROID)
    target_compile_definitions(pd_renderer PRIVATE PD_OPENGL_ES=1)

    # SDLActivity loads the native game entry point from libmain.so.
    add_library(main SHARED
        desktop/main.cpp
        platform/sdl/sdl_application.cpp
    )
    target_include_directories(main PRIVATE platform/sdl)
    target_link_libraries(main PRIVATE pd_core pd_renderer SDL3::SDL3 android log)
    set(PD_APP_TARGET main)
else()
    target_compile_definitions(pd_renderer PRIVATE PD_OPENGL_ES=0)

    add_executable(pd_desktop
        desktop/main.cpp
        platform/sdl/sdl_application.cpp
    )
    target_include_directories(pd_desktop PRIVATE platform/sdl)
    target_link_libraries(pd_desktop PRIVATE pd_core pd_renderer SDL3::SDL3)

    if(WIN32)
        target_link_libraries(pd_renderer PRIVATE opengl32)
        target_link_libraries(pd_desktop PRIVATE opengl32)
    endif()

    set(PD_APP_TARGET pd_desktop)
    set_target_properties(pd_desktop PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

if(MSVC)
    foreach(target pd_core pd_renderer ${PD_APP_TARGET})
        target_compile_options(${target} PRIVATE /W4 /permissive- /Zc:__cplusplus)
    endforeach()
else()
    foreach(target pd_core pd_renderer ${PD_APP_TARGET})
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()

include(CTest)
if(BUILD_TESTING AND NOT ANDROID)
    add_executable(pd_core_smoke core/tests/core_smoke.cpp)
    target_link_libraries(pd_core_smoke PRIVATE pd_core)
    add_test(NAME pd_core_smoke COMMAND pd_core_smoke)
endif()

if(ANDROID)
    message(STATUS "Permission Denied native target: Android arm64-v8a")
    message(STATUS "Renderer: OpenGL ES 2.0")
else()
    message(STATUS "Permission Denied native target: desktop")
endif()
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "SDL3 source: ${sdl3_SOURCE_DIR}")
