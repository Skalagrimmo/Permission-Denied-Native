# Native Engine Roadmap

## Phase 0 — Bootstrap
- [x] Create isolated native project.
- [x] Establish C++20 core and SDL/OpenGL boundaries.
- [x] Configure and build on Windows.
- [x] Launch the first native scene and initialize OpenGL shaders.
- [x] Add a core smoke test for movement, sprint, turning and fixed-step clamping.
- [ ] Verify keyboard input and movement visually on the desktop window.

## Phase 1 — Core parity
- [x] Port common vector/math helpers with regression tests.
- [x] Port common game and world data models.
- [ ] Port deterministic district generation and seeded RNG.
- [x] Port movement, collision and sliding resolution.
- [ ] Port enemy state model and patrol/chase/alert logic.
- [ ] Port weapons, damage, alarms and hacking.
- [ ] Add deterministic regression tests against documented Kotlin behavior.

## Phase 2 — Unified renderer
- [ ] Port tile/room geometry generation.
- [ ] Add native world mesh buffers and entity transforms.
- [ ] Add lighting, fog, materials and effects.
- [ ] Implement ASCII/CRT as renderer post-processing options.
- [ ] Add renderer profiling counters and frame-time telemetry.

## Phase 3 — Audio and persistence
- [ ] Port procedural synth to a native audio callback.
- [ ] Define versioned platform-neutral save format.
- [ ] Add save/load tests.

## Phase 4 — Android
- [ ] Install/configure Android NDK and CMake packages.
- [ ] Add SDL3 Android application integration.
- [ ] Add thin Kotlin Activity wrapper only if required by the application shell.
- [ ] Build arm64-v8a first; evaluate armeabi-v7a separately.
- [ ] Verify touch HUD, lifecycle pause/resume, audio focus and save migration.

## Phase 5 — Performance and release
- [ ] Profile Windows and Android with the same scene and simulation.
- [ ] Add quality tiers for low-end hardware.
- [ ] Produce reproducible desktop and APK release builds.
