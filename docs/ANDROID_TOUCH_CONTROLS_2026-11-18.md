# Android Touch Controls — 2026-11-18

## Result

Added the first touchscreen control layer to the native C++ Android build and verified ADB-injected touch events on a physical Samsung SM-J710F.

The native game loop remains shared. Touch input is translated into the same `pd::core::InputState` used by desktop keyboard controls; no gameplay movement logic was moved into Java.

## Layout

- Lower-left fixed virtual stick: forward/back and strafe.
- Right half of the screen: relative horizontal swipe to rotate the view.
- Lower-right circular button: hold to sprint.
- HUD is drawn by the existing full-screen GLSL fragment shader and appears only on Android.
- Stick uses a 0.10 radial deadzone with remapped output to avoid slow drift around its center.
- Touch look deltas are distributed across the pending fixed simulation steps, avoiding frame-rate-dependent camera sensitivity.

## Device verification

Device: Samsung SM-J710F, Android 10, Exynos 7870, Mali-T830.

Observed on device:

- The Android GLSL shader compiled and linked; no shader/link errors appeared in logcat.
- Both HUD controls rendered in landscape over the existing cyberpunk validation scene.
- ADB-injected left-stick gesture reached final axis `(0.00, 1.00)`.
- ADB-injected right-side swipe produced a sequence of non-zero `FINGER_MOTION` deltas; the scene's perspective grid visibly changed orientation.
- ADB-injected lower-right press generated sprint down/up events.
- The native C++ `core_smoke` suite was cross-compiled for arm64-v8a with NDK 28.2 and executed directly on the J710F. Process exited with code 0, including the new one-shot touch-look angle regression assertion.

These were software-injected touch events routed through the target phone's Android input stack. Direct hand-feel tuning on the touchscreen and simultaneous multi-finger testing remain to be done.

## Known limits

This is an input/HUD milestone, not yet a complete playable game. The rendered world is still the procedural validation scene; there is no generated district mesh, enemy AI, weapon/combat loop, hacking interaction, or gameplay audio. Player translation currently shifts the validation floor grid; full collision integration into the live game loop is still separate work.

## Next steps

1. Tune stick deadzone and look sensitivity by direct hand testing on the device.
2. Verify simultaneous move + look + sprint with multiple fingers.
3. Add pause/resume handling and confirm controls reset correctly after app focus loss.
4. Replace the validation scene with native generated world geometry, then connect movement to world collision.
