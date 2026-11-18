# Movement Controller Parity — 2026-11-18

Source: Permission-Denied-lab/app/src/main/java/com/example/engine/PlayerMovementController.kt

Native implementation:
- core/include/pd/core/movement.hpp
- core/src/movement.cpp

## Behavior preserved

- Defaults: player radius 0.28, walk 3.0, sprint 4.8, crouch 1.5, pitch range -45° to +45°.
- Input is normalized only when its magnitude exceeds 1.0; smaller analog values remain unchanged.
- Heading convention: yaw 0° moves forward along +Z; yaw 90° moves forward along +X; right strafe is clockwise from forward.
- Yaw wraps into [0°, 360°); pitch is clamped to the configured range.
- Player collision uses a circle against solid 1x1 tile AABBs. A strict distance comparison preserves the Kotlin edge behavior when a circle merely touches a tile.
- Movement resolves the X axis first and the Z axis second, allowing wall sliding.
- When an axis destination collides, four binary-search iterations find the furthest clear point along that axis.
- Final coordinates are clamped to the playable map bounds.

## Source behavior limitation

Despite the original Kotlin documentation calling this swept collision, the implementation checks the destination of each axis move before its four-step binary search. It does not test every point along the path. A sufficiently large single-step displacement can therefore cross a thin wall if the final destination is already clear. Native currently preserves this behavior for parity; simulation-scale collision tests use ordinary per-step displacements. A true swept test can be introduced later as an explicitly documented gameplay fix with new regression expectations.

## Regression coverage

The core smoke test checks default movement values, normalized and analog input, yaw-based forward direction, sprint displacement, view-angle wrap/clamp, circle-vs-tile collision, blocked movement, wall sliding and map-boundary clamping.

Invalid or dimensionally inconsistent native worlds are rejected safely by the controller; this guard prevents invalid grid access and does not alter valid-world behavior.
