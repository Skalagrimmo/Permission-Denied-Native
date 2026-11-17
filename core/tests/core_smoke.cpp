#include "pd/core/game.hpp"

#include <cassert>
#include <cmath>

int main() {
    using namespace pd::core;

    Game game;
    const auto initial = game.state();

    InputState input{};
    input.move_z = 1.0F;
    game.update(input, 1.0F / 60.0F);

    const auto after_forward = game.state();
    assert(std::abs(after_forward.player.x - initial.player.x) < 0.0001F);
    assert(after_forward.player.z > initial.player.z);
    assert(after_forward.simulation_ticks == 1);

    input = {};
    input.move_x = 1.0F;
    input.move_z = 1.0F;
    const auto before_diagonal = game.state();
    game.update(input, 1.0F / 60.0F);

    const auto after_diagonal = game.state();
    const float dx = after_diagonal.player.x - before_diagonal.player.x;
    const float dz = after_diagonal.player.z - before_diagonal.player.z;
    const float diagonal_distance = std::sqrt(dx * dx + dz * dz);
    assert(std::abs(diagonal_distance - 0.05F) < 0.001F);

    input = {};
    input.move_z = 1.0F;
    input.sprint = true;
    game.update(input, 1.0F / 60.0F);
    assert(std::abs(game.state().player.speed - 4.95F) < 0.001F);

    input = {};
    input.look_x = 1.0F;
    const float yaw_before = game.state().player.yaw_radians;
    game.update(input, 1.0F / 60.0F);
    assert(game.state().player.yaw_radians > yaw_before);

    const double elapsed_before_clamp = game.state().elapsed_seconds;
    game.update(InputState{}, 1.0F);
    const double clamped_step = game.state().elapsed_seconds - elapsed_before_clamp;
    assert(clamped_step <= 0.100001);
    assert(game.state().simulation_ticks == 5);

    return 0;
}
