# Permission Denied — Native Engine

Native C++ rewrite of the original Android/Kotlin prototype. The shared simulation and renderer are native; the Android Activity is a thin SDL lifecycle shell.

## Technical direction

- C++20 for shared game systems and simulation.
- SDL3 3.4.20 for windows, input, audio devices and platform lifecycle.
- OpenGL on desktop and OpenGL ES on Android.
- CMake as the single native build system.
- Minimal Java Activity only to host SDL on Android; no gameplay logic is placed in Java/Kotlin.

SDL3 is fetched and pinned by CMake FetchContent. Vendor source is not copied into this repository.

## Windows desktop build

Requirements: Visual Studio C++ desktop workload, CMake, Ninja, Git.

From a Visual Studio Developer PowerShell:

    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    .\build\bin\pd_desktop.exe

From a regular PowerShell, initialize the Visual Studio environment first:

    & "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build

## Android arm64 build

Requirements: JDK 21, Android SDK Platform 36, NDK 28.2.13676358, CMake 3.31.5 and the included Gradle wrapper.

Set the SDK path, then build from the Android project:

    $env:ANDROID_SDK_ROOT = "$env:LOCALAPPDATA\Android\Sdk"
    $env:ANDROID_HOME = $env:ANDROID_SDK_ROOT
    cd android
    .\gradlew.bat assembleDebug

When SDL3 3.4.20 source is already checked out locally, avoid a second FetchContent clone by supplying:

    .\gradlew.bat assembleDebug -PSDL3_SOURCE_DIR="C:/path/to/SDL"

The debug APK is written to:

    android/app/build/outputs/apk/debug/app-debug.apk

The Android target currently packages only `arm64-v8a`, uses minSdk 24, SDL3 shared library plus `libmain.so`, OpenGL ES 2.0 context, and landscape orientation. Desktop builds continue to use static SDL3.

## Tests

    cmake --build build --target pd_core_smoke
    ctest --test-dir build --output-on-failure

## Controls

Desktop keyboard controls: W/A/S/D move, Shift sprint, Left/Right arrows turn, Escape quit.

Android touchscreen controls:
- Lower-left virtual stick: the only movement input; forward/back and strafe are mapped independently of camera look.
- Entire right half: horizontal and vertical camera look only; it never generates movement input.
- Enlarged lower-right touch zone: hold to sprint. Touching it also keeps the right-side camera-look gesture active, so sprint and look can be combined.

Touch and keyboard input feed the same native `InputState`; touch-look deltas are applied on the fixed simulation step. The Android HUD is drawn by the shared GLSL renderer. The current scene is still a procedural renderer validation scene, not the finished game; generated world geometry, enemies, combat, hacking and gameplay audio remain future work.

## Layout

    core/       Platform-independent game simulation.
    renderer/   Shared OpenGL renderer and GL entry-point loader.
    platform/   SDL3 platform integration.
    desktop/    Desktop entry point.
    android/    SDL Activity, Gradle project and Android native target.
    assets/     Game assets and shader sources.
    docs/       Architecture, parity and build notes.

## Current milestone

0.1.0-native-bootstrap: SDL3 app lifecycle, OpenGL/OpenGL ES shader initialization, procedural cyberpunk test scene and fixed-step C++ simulation.

The original Kotlin project remains the behavioral reference while game systems are ported and verified incrementally.
