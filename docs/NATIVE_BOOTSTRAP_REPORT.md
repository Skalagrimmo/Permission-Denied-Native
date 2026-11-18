# Native Bootstrap Report — 2026-11-17

## Result

- Windows x64 configuration succeeded with MSVC 19.51.36256, CMake 4.3.1 and Ninja 1.13.2.
- SDL3 3.4.20 was fetched from its official repository and built as a static library.
- The C++20 game core, OpenGL renderer and SDL3 desktop application compiled and linked.
- The desktop executable launched successfully.
- Runtime log confirmed SDL3 initialization, OpenGL function loading, GLSL shader compilation and program linking.
- The launch test entered the SDL event/render loop after successful initialization.
- `pd_core_smoke` passed under CTest: 1/1 tests passed, covering forward movement, diagonal normalization, sprint speed, yaw updates and delta-time clamping.

## Executable

build/bin/pd_desktop.exe

## First scene

The bootstrap renderer draws a procedural cyberpunk skyline, perspective floor grid, horizon glow, scanlines, vignette and a center reticle. The C++ simulation updates through a fixed 60 Hz step.

## Controls

- W / A / S / D — move.
- Left Shift — sprint.
- Left / Right arrows — turn.
- Escape — exit.

## Known scope limits

This is a native engine bootstrap, not a full port of the original game yet. The current scene is a shader-based renderer validation scene. Original district generation, collision, enemy AI, weapons, hacking, procedural audio, save persistence and the Android wrapper remain on the porting roadmap.

## Phase 1 progress — 2026-11-18

- Added `core/include/pd/core/math.hpp` with the Kotlin `Vector3` and `MathUtils` behavior.
- Preserved the small-vector normalization threshold (`0.0001`), clamped interpolation parameter and signed angle-wrap interval `[-180°, 180°)`.
- Extended `pd_core_smoke` with vector arithmetic, magnitude, distance, normalization, interpolation and angle-wrap regression checks.
- Configured the smoke-test target to keep `assert` checks enabled in Release builds.
- Recorded the source-to-native behavior mapping in `docs/CORE_MATH_PARITY.md`.

## Original project preservation

No files in Permission-Denied-lab were changed. The new native work is isolated in Permission-Denied-Native.
