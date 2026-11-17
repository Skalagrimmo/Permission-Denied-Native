# Permission Denied — Native Engine

Native C++ rewrite of the original Android/Kotlin prototype.

## Technical direction

- C++20 for the shared simulation and game systems.
- SDL3 for platform services: window, input, audio devices and lifecycle.
- OpenGL for the shared renderer, with desktop OpenGL and Android OpenGL ES shader paths.
- Kotlin only as a future Android application wrapper.
- CMake as the single native build system.

SDL is fetched and pinned by CMake FetchContent at release-3.4.20. Vendor source is not copied into this repository.

## Windows build

Requirements:
- Visual Studio C++ desktop workload (MSVC x64).
- CMake and Ninja.
- Git and internet access on the first configure to fetch SDL3.

From a Visual Studio Developer PowerShell:

    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    .\build\bin\pd_desktop.exe

From a regular PowerShell, initialize the Visual Studio environment first:

    & "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build

## Tests

    cmake --build build --target pd_core_smoke
    ctest --test-dir build --output-on-failure

## Controls

- W, A, S, D — move.
- Shift — sprint.
- Left / Right arrows — turn.
- Escape — quit.

## Layout

    core/       Platform-independent game simulation.
    renderer/   Shared OpenGL renderer and GL entry-point loader.
    platform/   SDL3 platform integration.
    desktop/    Windows/Linux desktop entry point.
    android/    Reserved for Android wrapper and NDK build.
    assets/     Game assets and shader sources.
    docs/       Architecture and migration notes.

## Current milestone

0.1.0-native-bootstrap: SDL3 window, OpenGL context, procedural cyberpunk scene and fixed-step C++ simulation.

The existing Kotlin project remains the reference implementation while systems are ported and verified one at a time.
