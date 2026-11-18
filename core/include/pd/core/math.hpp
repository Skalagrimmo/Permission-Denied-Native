#pragma once

#include <cmath>

namespace pd::core {

struct Vector3 final {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;

    constexpr Vector3() noexcept = default;
    constexpr Vector3(float x_value, float y_value, float z_value) noexcept
        : x(x_value), y(y_value), z(z_value) {}

    constexpr Vector3& set(float new_x, float new_y, float new_z) noexcept {
        x = new_x;
        y = new_y;
        z = new_z;
        return *this;
    }

    [[nodiscard]] constexpr Vector3 add(const Vector3& other) const noexcept {
        return {x + other.x, y + other.y, z + other.z};
    }

    [[nodiscard]] constexpr Vector3 sub(const Vector3& other) const noexcept {
        return {x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr Vector3 scale(float factor) const noexcept {
        return {x * factor, y * factor, z * factor};
    }

    [[nodiscard]] float length() const noexcept {
        return std::sqrt(length_squared());
    }

    [[nodiscard]] constexpr float length_squared() const noexcept {
        return x * x + y * y + z * z;
    }

    [[nodiscard]] float distance_to(const Vector3& other) const noexcept {
        const float dx = x - other.x;
        const float dy = y - other.y;
        const float dz = z - other.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    [[nodiscard]] float distance_2d_to(float target_x, float target_z) const noexcept {
        const float dx = x - target_x;
        const float dz = z - target_z;
        return std::sqrt(dx * dx + dz * dz);
    }

    // Kotlin parity: magnitudes <= 0.0001 are treated as zero.
    [[nodiscard]] Vector3 normalize() const noexcept {
        const float magnitude = length();
        if (magnitude > 0.0001F) {
            return {x / magnitude, y / magnitude, z / magnitude};
        }
        return {};
    }
};

namespace math {

inline constexpr float pi = 3.14159265358979323846F;
inline constexpr float two_pi = 2.0F * pi;
inline constexpr float deg_to_rad = pi / 180.0F;
inline constexpr float rad_to_deg = 180.0F / pi;

[[nodiscard]] constexpr float clamp(float value, float minimum, float maximum) noexcept {
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

[[nodiscard]] constexpr float lerp(float a, float b, float t) noexcept {
    return a + (b - a) * clamp(t, 0.0F, 1.0F);
}

// Result interval matches the Kotlin implementation: [-180, 180).
[[nodiscard]] inline float angle_difference(float a, float b) noexcept {
    float difference = std::fmod(b - a + 180.0F, 360.0F) - 180.0F;
    if (difference < -180.0F) {
        difference += 360.0F;
    }
    return difference;
}

[[nodiscard]] inline float distance_2d(
    float x1, float z1, float x2, float z2) noexcept {
    const float dx = x1 - x2;
    const float dz = z1 - z2;
    return std::sqrt(dx * dx + dz * dz);
}

} // namespace math
} // namespace pd::core
