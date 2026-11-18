#pragma once

#include "pd/core/world.hpp"

#include <utility>

namespace pd::core {

class PlayerMovementController final {
public:
    explicit PlayerMovementController(
        float player_radius = 0.28F,
        float walk_speed = 3.0F,
        float sprint_speed = 4.8F,
        float crouch_speed = 1.5F,
        float min_pitch = -45.0F,
        float max_pitch = 45.0F) noexcept;

    [[nodiscard]] std::pair<float, float> normalize_input(
        float raw_x, float raw_z) const noexcept;

    [[nodiscard]] std::pair<float, float> compute_velocity(
        float move_x,
        float move_z,
        float yaw_degrees,
        float speed,
        float delta_seconds) const noexcept;

    [[nodiscard]] std::pair<float, float> update_view_angles(
        float current_yaw,
        float current_pitch,
        float delta_yaw,
        float delta_pitch) const noexcept;

    [[nodiscard]] std::pair<float, float> resolve_grid_movement(
        float start_x,
        float start_z,
        float velocity_x,
        float velocity_z,
        const GeneratedWorld& world) const noexcept;

    [[nodiscard]] bool check_collision_with_world(
        float x, float z, const GeneratedWorld& world) const noexcept;

    [[nodiscard]] float player_radius() const noexcept { return player_radius_; }
    [[nodiscard]] float walk_speed() const noexcept { return walk_speed_; }
    [[nodiscard]] float sprint_speed() const noexcept { return sprint_speed_; }
    [[nodiscard]] float crouch_speed() const noexcept { return crouch_speed_; }
    [[nodiscard]] float min_pitch() const noexcept { return min_pitch_; }
    [[nodiscard]] float max_pitch() const noexcept { return max_pitch_; }

private:
    [[nodiscard]] static bool circle_intersects_tile_aabb(
        float center_x,
        float center_z,
        float radius,
        int tile_x,
        int tile_z) noexcept;

    [[nodiscard]] float find_closest_clear_position_on_axis(
        float start_value,
        float target_value,
        float other_axis_value,
        bool is_x_axis,
        const GeneratedWorld& world) const noexcept;

    float player_radius_;
    float walk_speed_;
    float sprint_speed_;
    float crouch_speed_;
    float min_pitch_;
    float max_pitch_;
};

} // namespace pd::core
