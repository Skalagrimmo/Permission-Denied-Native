# Android target

The Android shell hosts the shared native C++ engine through SDL3. The Java Activity only supplies Android lifecycle integration; simulation and rendering remain in C++.

## Build

Requirements: JDK 21, Android SDK Platform 36, NDK 28.2.13676358, CMake 3.31.5.

    $env:ANDROID_SDK_ROOT = "$env:LOCALAPPDATA\Android\Sdk"
    $env:ANDROID_HOME = $env:ANDROID_SDK_ROOT
    .\gradlew.bat assembleDebug

To reuse a local SDL3 3.4.20 source checkout:

    .\gradlew.bat assembleDebug -PSDL3_SOURCE_DIR="C:/path/to/SDL"

Output:

    app/build/outputs/apk/debug/app-debug.apk

## Current target

- ABI: arm64-v8a
- minSdk: 24
- Target SDK: 35
- SDL3: shared `libSDL3.so`, plus app entry point `libmain.so`
- Renderer: OpenGL ES 2.0 context
- Orientation: LandscapeLeft / LandscapeRight

## Verified device smoke test — 2026-11-18

- Samsung SM-J710F, Android 10
- Exynos 7870, ARM Mali-T830 GPU (device EGL stack reports OpenGL ES 3.2 support)
- APK installed and launched over ADB
- SDL main entry point resolved from `libmain.so`
- SDL created the Android surface and initialized the OpenGL ES renderer
- Procedural cyberpunk validation scene rendered in landscape

This is a renderer/lifecycle and initial touch-input smoke test, not a full gameplay port. The current HUD provides a lower-left movement stick, right-side swipe look, and a lower-right hold-to-sprint button. These controls feed the same native input state as desktop keyboard controls. ADB-injected finger events were verified on the physical SM-J710F; direct hand-feel tuning and multi-device testing remain open. Gameplay geometry, world generation, enemies, combat, pause/resume validation and audio focus remain on the roadmap.
