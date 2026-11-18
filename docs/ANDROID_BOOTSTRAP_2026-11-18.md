# Android Bootstrap Report — 2026-11-18

## Result

Built, installed and launched the first arm64-v8a APK from the native C++ repository on a physical Samsung SM-J710F.

The Android shell uses SDL3 3.4.20's Java Activity sources and a minimal app-owned Activity. The simulation and renderer remain native C++.

## Build configuration

- JDK 21
- Android SDK Platform 36
- Android NDK 28.2.13676358
- CMake 3.31.5
- Gradle wrapper 9.3.1 / Android Gradle Plugin 9.1.1
- ABI: arm64-v8a
- minSdk 24, targetSdk 35
- SDL3 is a shared library on Android (`libSDL3.so`) and remains static on desktop.
- `libmain.so` exports the SDL Android entry point via SDL3's `SDL_main.h`.
- Renderer requests an OpenGL ES 2.0 context; the test device reports a Mali-T830 / OpenGL ES 3.2 graphics stack.
- Android orientation is restricted to LandscapeLeft and LandscapeRight via `SDL_HINT_ORIENTATIONS`.

## Device smoke-test evidence

Device reported by ADB:

- Model: SM-J710F
- Android: 10
- Hardware platform: Exynos 7870
- GPU: ARM Mali-T830

Runtime log confirmed:

- SDL Java lifecycle reached onCreate/onResume.
- SDL resolved `SDL_main` from `libmain.so`.
- `OpenGL renderer ready. Procedural cyberpunk test scene loaded.`
- `Permission Denied native engine initialized.`
- Activity remained the resumed foreground activity.
- A screenshot showed the procedural horizon, perspective floor grid and center reticle in landscape.

The APK contains the expected arm64 native libraries: `libSDL3.so`, `libc++_shared.so` and `libmain.so`.

## Integration findings

1. Android SDL Activity expects `libSDL3.so`; static SDL linking alone is not sufficient for this stock Java loader. CMake now builds shared SDL3 on Android and static SDL3 for desktop.
2. Android SDL Activity resolves `SDL_main`, not the desktop `main` symbol. The entry point includes SDL3's `SDL_main.h` on Android only.
3. Android requests an OpenGL ES profile, while desktop keeps its OpenGL 2.1 context.
4. SDL permits all orientations by default for a resizable window; the native app now sets the SDL orientation hint to landscape.

## Environment-specific toolchain repair

The test laptop's installed NDK 28.2.13676358 contained a malformed preprocessor line in libc++'s `__ranges/access.h`:

`#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER!`

The local SDK header was repaired to remove the stray exclamation mark before compilation. This was a change to the laptop's installed Android SDK only, not to repository source. A clean SDK installation should be checked for this local toolchain anomaly before using the same NDK package.

## Current limits

This is still the native engine bootstrap scene, not a full gameplay build. It has no touch HUD, touchscreen camera control, gameplay world geometry, enemies, weapons, hacking or audio gameplay. Existing keyboard mappings remain useful with an external keyboard. The next Android work should add touch movement/look controls, test lifecycle pause/resume, and then feed generated world geometry into the renderer.
