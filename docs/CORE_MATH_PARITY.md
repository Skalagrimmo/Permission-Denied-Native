# Core Math Parity — 2026-11-18

Source: Permission-Denied-lab/app/src/main/java/com/example/engine/Vector3.kt

Native implementation: core/include/pd/core/math.hpp

## Ported behavior

| Kotlin API | Native API | Behavior preserved |
|---|---|---|
| Vector3.set | Vector3::set | Mutates the current vector and returns it for chaining. |
| add, sub, scale | add, sub, scale | Return new vector values; do not mutate the source. |
| length, lengthSquared | length, length_squared | Euclidean magnitude and squared magnitude. |
| distanceTo | distance_to | Three-dimensional Euclidean distance. |
| distance2DTo | distance_2d_to | Distance on the X/Z plane. |
| normalize | normalize | Returns a new unit vector; magnitudes at or below 0.0001 return zero. |
| MathUtils.clamp | math::clamp | Same ordered comparisons. |
| MathUtils.lerp | math::lerp | Clamps interpolation parameter to [0, 1]. |
| MathUtils.angleDifference | math::angle_difference | Signed shortest turn in [-180°, 180°). |
| MathUtils.distance2D | math::distance_2d | Euclidean X/Z-plane distance. |

## Verification

pd_core_smoke now covers vector arithmetic, magnitude, distance, normalization edge cases, interpolation clamping, and angle wrapping in addition to the original movement smoke checks.

This is a math-layer port only. World models, district generation, and collision resolution remain separate migration steps.
