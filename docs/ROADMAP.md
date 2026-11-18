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
- [x] Add a depth-tested perspective wireframe corridor as the first native vector-scene prototype.
- [x] Reimplement the Nano-punk camera-space projection approach in the C++ renderer.
- [x] Add world-space 5×7 ASCII signage and vector touch-HUD accents.
- [ ] Port tile/room geometry generation.
- [ ] Add reusable world mesh buffers and entity transforms.
- [ ] Add lighting, fog, materials and effects.
- [ ] Implement optional ASCII/CRT surface and post-processing modes.
- [ ] Add renderer profiling counters and frame-time telemetry.

## Phase 3 — Audio and persistence
- [ ] Port procedural synth to a native audio callback.
- [ ] Define versioned platform-neutral save format.
- [ ] Add save/load tests.

## Phase 4 — Android
- [x] Install/configure Android NDK and CMake packages.
- [x] Add SDL3 Android application integration with a thin Java Activity shell.
- [x] Build and install an arm64-v8a APK on a physical Samsung SM-J710F.
- [x] Verify SDL_main resolution and OpenGL ES renderer startup in landscape.
- [x] Add Android touch HUD with exclusive left-stick movement, full right-half two-axis look, and enlarged sprint target.
- [x] Verify left-only movement, right-only horizontal look, and simultaneous sprint + look in the sprint target on a physical SM-J710F.
- [x] Correct world-grid projection signs so the scene moves opposite to camera/player motion.
- [x] Build, install and launch the vector/ASCII hybrid renderer on the physical SM-J710F.
- [x] Remove the duplicate touch-stick axis inversion after integrating the vector camera; preserve the right-side look path.
- [ ] Tune touch deadzones/sensitivity from direct hand testing and test multi-touch on additional devices.
- [ ] Verify lifecycle pause/resume, audio focus and save migration.
- [ ] Evaluate armeabi-v7a separately.

## Phase 5 — Performance and release
- [ ] Profile Windows and Android with the same scene and simulation.
- [ ] Add quality tiers for low-end hardware.
- [ ] Produce reproducible desktop and APK release builds.
