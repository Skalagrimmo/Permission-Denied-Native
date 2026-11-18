#pragma once
#include <cstdint>

namespace pd::core {
struct InputState {
    float move_x = 0.0F;
    float move_z = 0.0F;
    float look_x = 0.0F;
    float look_delta_radians = 0.0F; // One-shot relative-look input, used by touch.
    bool sprint = false;
};

struct PlayerState {
    float x = 2.5F;
    float z = 2.5F;
    float yaw_radians = 0.0F;
    float speed = 3.0F;
};

struct GameState {
    PlayerState player{};
    double elapsed_seconds = 0.0;
    std::uint64_t simulation_ticks = 0;
};

class Game final {
public:
    Game() = default;
    void reset() noexcept;
    void update(const InputState& input, float delta_seconds) noexcept;
    [[nodiscard]] const GameState& state() const noexcept { return state_; }

private:
    GameState state_{};
};
} // namespace pd::core
