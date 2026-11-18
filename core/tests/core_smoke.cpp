#include "pd/core/game.hpp"
#include "pd/core/math.hpp"

// Keep assertions enabled for this test translation unit in Release builds.
#ifdef NDEBUG
#undef NDEBUG
#endif
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

    // Kotlin Vector3 / MathUtils parity checks.
    Vector3 vector{3.0F, 4.0F, 12.0F};
    assert(std::abs(vector.length() - 13.0F) < 0.0001F);
    assert(std::abs(vector.length_squared() - 169.0F) < 0.0001F);
    assert(std::abs(vector.distance_to(Vector3{}) - 13.0F) < 0.0001F);
    assert(std::abs(vector.distance_2d_to(0.0F, 0.0F) - std::sqrt(153.0F)) < 0.0001F);

    const Vector3 sum = vector.add(Vector3{1.0F, 2.0F, 3.0F});
    assert(std::abs(sum.x - 4.0F) < 0.0001F);
    assert(std::abs(sum.y - 6.0F) < 0.0001F);
    assert(std::abs(sum.z - 15.0F) < 0.0001F);

    const Vector3 difference = vector.sub(Vector3{1.0F, 1.0F, 2.0F});
    assert(std::abs(difference.x - 2.0F) < 0.0001F);
    assert(std::abs(difference.y - 3.0F) < 0.0001F);
    assert(std::abs(difference.z - 10.0F) < 0.0001F);

    const Vector3 scaled = vector.scale(2.0F);
    assert(std::abs(scaled.length() - 26.0F) < 0.0001F);
    assert(std::abs(vector.normalize().length() - 1.0F) < 0.0001F);
    assert(vector.normalize().x > 0.0F);
    assert(Vector3{}.normalize().length_squared() == 0.0F);
    const Vector3 tiny_vector{0.00005F, 0.0F, 0.0F};
    assert(tiny_vector.normalize().length_squared() == 0.0F);

    Vector3 mutable_vector{};
    Vector3& chained = mutable_vector.set(2.0F, 3.0F, 4.0F);
    assert(&chained == &mutable_vector);
    assert(mutable_vector.x == 2.0F && mutable_vector.y == 3.0F && mutable_vector.z == 4.0F);

    assert(std::abs(math::pi - 3.14159265F) < 0.000001F);
    assert(std::abs(math::two_pi - 6.28318530F) < 0.000001F);
    assert(std::abs(math::deg_to_rad * 180.0F - math::pi) < 0.000001F);
    assert(std::abs(math::rad_to_deg * math::pi - 180.0F) < 0.0001F);
    assert(math::clamp(-2.0F, 0.0F, 1.0F) == 0.0F);
    assert(math::clamp(2.0F, 0.0F, 1.0F) == 1.0F);
    assert(math::clamp(0.25F, 0.0F, 1.0F) == 0.25F);
    assert(math::lerp(10.0F, 20.0F, 0.5F) == 15.0F);
    assert(math::lerp(10.0F, 20.0F, -1.0F) == 10.0F);
    assert(math::lerp(10.0F, 20.0F, 2.0F) == 20.0F);
    assert(std::abs(math::angle_difference(350.0F, 10.0F) - 20.0F) < 0.0001F);
    assert(std::abs(math::angle_difference(10.0F, 350.0F) + 20.0F) < 0.0001F);
    assert(math::angle_difference(0.0F, 180.0F) == -180.0F);
    assert(std::abs(math::distance_2d(0.0F, 0.0F, 3.0F, 4.0F) - 5.0F) < 0.0001F);

    return 0;
}
