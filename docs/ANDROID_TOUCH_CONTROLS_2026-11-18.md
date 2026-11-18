# Android Touch Controls — 2026-11-18

## Control layout

- Lower-left fixed virtual stick: forward/back and strafe.
- Right half of the screen: relative swipe for both yaw and pitch.
- Lower-right circular sprint button with a deliberately generous touch target.
- HUD is drawn by the existing full-screen GLSL fragment shader and appears only on Android.
- Stick uses a 0.10 radial deadzone with remapped output to avoid slow drift around its center.
- Touch yaw and pitch deltas are distributed across pending fixed simulation steps, avoiding frame-rate-dependent camera sensitivity.
- Sprint touch radius is 0.23 screen-height units (about 166 px radius / 331 px diameter at 720p); the visible button is smaller than its hit area.

## Direction correction

Initial device feedback identified that the scene appeared to move in the same direction as the player/camera. The touch axes themselves were already sending the intended direction; the inversion was in the procedural floor-grid projection. Player translation and yaw offsets in the shader have therefore been changed to subtract camera movement from static-world coordinates. The movement input mapping was intentionally left unchanged.

Vertical swipe was added as pitch input. An upward swipe raises the view; pitch is clamped to +/-45 degrees in the native game state and moves the horizon in the shader.

## Device verification

Device: Samsung SM-J710F, Android 10, Exynos 7870, Mali-T830.

Previously verified on device:
- Android shader compiles and links.
- Left-stick events reach the full forward axis.
- Right-side swipe emits non-zero FINGER_MOTION deltas.
- Sprint down/up events are received.
- Core smoke suite was compiled for arm64-v8a and executed on device with exit code 0.

Current revision rebuilt and installed on the SM-J710F:
- A sprint tap at screen coordinate (1000, 580), which was outside the previous 0.135-radius hit area, now triggers sprint down/up inside the expanded 0.23-radius area.
- A diagonal right-side swipe (up and right) changes yaw and pitch; the captured screen shows the horizon moving lower after looking upward.
- The GLSL renderer starts without shader/link errors.
- The updated arm64 core smoke suite executes on-device with exit code 0, including pitch clamping at +/-45 degrees.

## Known limits

This remains an input/HUD milestone, not a complete game. The rendered world is a procedural validation scene; generated district geometry, collision integration in the live loop, enemies, weapon/combat systems, hacking and gameplay audio are still future work. Direct hand-feel tuning and simultaneous multi-finger testing remain open.
