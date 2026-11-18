#include "pd/core/movement.hpp"

#include "pd/core/math.hpp"

#include <algorithm>
#include <cmath>

namespace pd::core {

PlayerMovementController::PlayerMovementController(
    float player_radius,
    float walk_speed,
    float sprint_speed,
    float crouch_speed,
    float min_pitch,
    float max_pitch) noexcept
    : player_radius_(player_radius),
      walk_speed_(walk_speed),
      sprint_speed_(sprint_speed),
      crouch_speed_(crouch_speed),
      min_pitch_(min_pitch),
      max_pitch_(max_pitch) {}

std::pair<float, float> PlayerMovementController::normalize_input(
    float raw_x, float raw_z) const noexcept {
    const float magnitude = std::sqrt(raw_x * raw_x + raw_z * raw_z);
    if (magnitude > 1.0F) {
        return {raw_x / magnitude, raw_z / magnitude};
    }
    return {raw_x, raw_z};
}

std::pair<float, float> PlayerMovementController::compute_velocity(
    float move_x,
    float move_z,
    float yaw_degrees,
    float speed,
    float delta_seconds) const noexcept {
    const auto [normalized_x, normalized_z] = normalize_input(move_x, move_z);
    const float yaw_radians = yaw_degrees * math::deg_to_rad;

    const float forward_x = std::sin(yaw_radians);
    const float forward_z = std::cos(yaw_radians);
    const float right_x = std::cos(yaw_radians);
    const float right_z = -std::sin(yaw_radians);

    const float velocity_x =
        (forward_x * normalized_z + right_x * normalized_x) * speed * delta_seconds;
    const float velocity_z =
        (forward_z * normalized_z + right_z * normalized_x) * speed * delta_seconds;

    return {velocity_x, velocity_z};
}

std::pair<float, float> PlayerMovementController::update_view_angles(
    float current_yaw,
    float current_pitch,
    float delta_yaw,
    float delta_pitch) const noexcept {
    float new_yaw = std::fmod(current_yaw + delta_yaw, 360.0F);
    if (new_yaw < 0.0F) {
        new_yaw += 360.0F;
    }

    const float new_pitch = math::clamp(
        current_pitch + delta_pitch, min_pitch_, max_pitch_);
    return {new_yaw, new_pitch};
}

bool PlayerMovementController::circle_intersects_tile_aabb(
    float center_x,
    float center_z,
    float radius,
    int tile_x,
    int tile_z) noexcept {
    const float tile_min_x = static_cast<float>(tile_x);
    const float tile_max_x = tile_min_x + 1.0F;
    const float tile_min_z = static_cast<float>(tile_z);
    const float tile_max_z = tile_min_z + 1.0F;

    const float closest_x = std::clamp(center_x, tile_min_x, tile_max_x);
    const float closest_z = std::clamp(center_z, tile_min_z, tile_max_z);
    const float distance_x = center_x - closest_x;
    const float distance_z = center_z - closest_z;
    const float distance_squared = distance_x * distance_x + distance_z * distance_z;

    // Strict comparison intentionally matches the Kotlin implementation:
    // a circle touching the tile boundary is not considered penetrating it.
    return distance_squared < radius * radius;
}

bool PlayerMovementController::check_collision_with_world(
    float x, float z, const GeneratedWorld& world) const noexcept {
    if (!world.valid()) {
        return false;
    }

    const int min_tile_x = std::max(
        0, static_cast<int>(std::floor(x - player_radius_)));
    const int max_tile_x = std::min(
        world.width - 1, static_cast<int>(std::floor(x + player_radius_)));
    const int min_tile_z = std::max(
        0, static_cast<int>(std::floor(z - player_radius_)));
    const int max_tile_z = std::min(
        world.height - 1, static_cast<int>(std::floor(z + player_radius_)));

    for (int tile_x = min_tile_x; tile_x <= max_tile_x; ++tile_x) {
        for (int tile_z = min_tile_z; tile_z <= max_tile_z; ++tile_z) {
            const TileType tile = world.grid.at(tile_x, tile_z);
            if (tile_type_info(tile).is_solid &&
                circle_intersects_tile_aabb(x, z, player_radius_, tile_x, tile_z)) {
                return true;
            }
        }
    }
    return false;
}

float PlayerMovementController::find_closest_clear_position_on_axis(
    float start_value,
    float target_value,
    float other_axis_value,
    bool is_x_axis,
    const GeneratedWorld& world) const noexcept {
    float low = 0.0F;
    float high = 1.0F;
    float best_position = start_value;

    // Four bisection iterations match PlayerMovementController.kt.
    for (int iteration = 0; iteration < 4; ++iteration) {
        const float midpoint = (low + high) * 0.5F;
        const float test_position =
            start_value + (target_value - start_value) * midpoint;
        const float test_x = is_x_axis ? test_position : other_axis_value;
        const float test_z = is_x_axis ? other_axis_value : test_position;

        if (!check_collision_with_world(test_x, test_z, world)) {
            best_position = test_position;
            low = midpoint;
        } else {
            high = midpoint;
        }
    }
    return best_position;
}

std::pair<float, float> PlayerMovementController::resolve_grid_movement(
    float start_x,
    float start_z,
    float velocity_x,
    float velocity_z,
    const GeneratedWorld& world) const noexcept {
    if (!world.valid()) {
        return {start_x, start_z};
    }

    float resolved_x = start_x;
    float resolved_z = start_z;

    if (std::abs(velocity_x) > 0.0001F) {
        const float target_x = std::clamp(
            resolved_x + velocity_x,
            player_radius_,
            static_cast<float>(world.width) - player_radius_);
        if (!check_collision_with_world(target_x, resolved_z, world)) {
            resolved_x = target_x;
        } else {
            resolved_x = find_closest_clear_position_on_axis(
                resolved_x, target_x, resolved_z, true, world);
        }
    }

    if (std::abs(velocity_z) > 0.0001F) {
        const float target_z = std::clamp(
            resolved_z + velocity_z,
            player_radius_,
            static_cast<float>(world.height) - player_radius_);
        if (!check_collision_with_world(resolved_x, target_z, world)) {
            resolved_z = target_z;
        } else {
            resolved_z = find_closest_clear_position_on_axis(
                resolved_z, target_z, resolved_x, false, world);
        }
    }

    return {
        std::clamp(resolved_x, player_radius_,
                   static_cast<float>(world.width) - player_radius_),
        std::clamp(resolved_z, player_radius_,
                   static_cast<float>(world.height) - player_radius_)
    };
}

} // namespace pd::core
