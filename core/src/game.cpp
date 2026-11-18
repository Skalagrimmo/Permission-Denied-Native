#include "pd/core/game.hpp"
#include <algorithm>
#include <cmath>

namespace pd::core {
namespace {
constexpr float kWalkSpeed = 3.0F;
constexpr float kSprintMultiplier = 1.65F;
constexpr float kTurnSpeed = 2.2F;
constexpr float kMaxDeltaSeconds = 0.1F;
constexpr float kTwoPi = 6.28318530717958647692F;
}

void Game::reset() noexcept {
    state_ = GameState{};
}

void Game::update(const InputState& input, float delta_seconds) noexcept {
    const float dt = std::clamp(delta_seconds, 0.0F, kMaxDeltaSeconds);
    if (dt <= 0.0F) return;

    state_.player.yaw_radians += input.look_x * kTurnSpeed * dt + input.look_delta_radians;
    if (state_.player.yaw_radians >= kTwoPi) {
        state_.player.yaw_radians = std::fmod(state_.player.yaw_radians, kTwoPi);
    } else if (state_.player.yaw_radians < 0.0F) {
        state_.player.yaw_radians = std::fmod(state_.player.yaw_radians, kTwoPi) + kTwoPi;
    }

    float local_x = input.move_x;
    float local_z = input.move_z;
    const float input_length_sq = local_x * local_x + local_z * local_z;
    if (input_length_sq > 1.0F) {
        const float inverse_length = 1.0F / std::sqrt(input_length_sq);
        local_x *= inverse_length;
        local_z *= inverse_length;
    }

    const float cosine = std::cos(state_.player.yaw_radians);
    const float sine = std::sin(state_.player.yaw_radians);
    const float speed = kWalkSpeed * (input.sprint ? kSprintMultiplier : 1.0F);
    const float world_x = local_x * cosine + local_z * sine;
    const float world_z = -local_x * sine + local_z * cosine;

    state_.player.x += world_x * speed * dt;
    state_.player.z += world_z * speed * dt;
    state_.player.speed = speed;
    state_.elapsed_seconds += static_cast<double>(dt);
    ++state_.simulation_ticks;
}
} // namespace pd::core
