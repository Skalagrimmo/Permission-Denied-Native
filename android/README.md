# Android target

This directory is reserved for the Android application shell.

The game simulation and renderer remain in the shared native C++ targets. The Android build will use the Android NDK, CMake and SDL3's Android project integration, with a minimal Kotlin Activity only for Android lifecycle and platform services.

Initial ABI target: arm64-v8a. Additional ABIs will be evaluated after the first Android build.
