#include "pd/core/game.hpp"
#include "pd/core/math.hpp"
#include "pd/core/models.hpp"
#include "pd/core/world.hpp"

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

    // Kotlin GameModels / WorldGenerator data-contract parity checks.
    const Weapon pistol = Weapon::create_default_pistol();
    assert(pistol.type == WeaponType::silenced_pistol);
    assert(pistol.ammo_in_mag == 12 && pistol.max_mag == 12 && pistol.reserve_ammo == 36);
    assert(pistol.damage == 38 && pistol.fire_rate_ms == 280 && pistol.reload_time_ms == 1200);
    assert(pistol.can_fire());
    Weapon reloading_pistol = pistol;
    reloading_pistol.is_reloading = true;
    assert(!reloading_pistol.can_fire());
    reloading_pistol.is_reloading = false;
    reloading_pistol.ammo_in_mag = 0;
    assert(!reloading_pistol.can_fire());
    assert(Weapon::create_stunner().damage == 100);
    assert(Weapon::create_smg().fire_rate_ms == 95);
    assert(Weapon::create_emp_grenade().ammo_in_mag == 3);
    assert(Weapon::create_cyberdeck().ammo_in_mag == 100);
    assert(Weapon::create_lockpick().ammo_in_mag == 5);
    assert(weapon_type_info(WeaponType::arc_stunner).is_lethal == false);
    assert(std::abs(weapon_type_info(WeaponType::cyber_smg).base_noise_radius - 18.0F) < 0.0001F);

    const AugmentationInfo dash = augmentation_info(AugmentationType::dash_thrusters);
    assert(dash.title == "Kinetic Thrusters");
    assert(std::abs(dash.energy_cost_instant - 25.0F) < 0.0001F);
    assert(dash.cooldown_ms == 2500);
    assert(augmentation_info(AugmentationType::subdermal_plating).cooldown_ms == 0);

    InventoryItem medkit{"stim_1", ItemType::health_stim, "Medi-Stim Pack", "Restores 40 Health", 2};
    assert(medkit.count == 2 && medkit.type == ItemType::health_stim);

    const Enemy boss{42, EnemyType::aegis_paladin, 12.0F, 0.5F, 15.0F, 180.0F};
    assert(boss.hp == 400 && boss.armor == 250);
    assert(boss.state == EnemyAiState::patrol);
    assert(boss.target_x == 12.0F && boss.target_z == 15.0F);
    assert(std::abs(enemy_type_info(EnemyType::security_drone).speed - 3.4F) < 0.0001F);
    assert(enemy_type_info(EnemyType::surveillance_camera).max_armor == 0);

    const TileTypeInfo glass = tile_type_info(TileType::wall_glass);
    assert(glass.is_solid && !glass.blocks_sight && glass.glyph == '|');
    const TileTypeInfo terminal = tile_type_info(TileType::terminal);
    assert(terminal.is_solid && !terminal.blocks_sight && terminal.glyph == '$');
    assert(!tile_type_info(TileType::laser_tripwire).is_solid);
    assert(tile_type_info(TileType::ghost_index_mainframe).glyph == '@');

    const DistrictInfo district_three = district_info(DistrictId::district_03);
    assert(district_three.level == 3 && district_three.size == 12);
    assert(district_three.title == "District 03: Industrial Core");
    assert(ending_info(EndingChoice::destroy).score_bonus == 6000);

    MissionReport report{};
    report.ending_choice = EndingChoice::leak;
    report.is_victory = true;
    assert(report.ending_choice == EndingChoice::leak && report.is_victory);

    Grid2D<int> x_major_grid{3, 2, -1};
    x_major_grid(1, 0) = 17;
    x_major_grid(2, 1) = 29;
    assert(x_major_grid.index(1, 0) == 2);
    assert(x_major_grid.index(2, 1) == 5);
    assert(x_major_grid.cells()[2] == 17 && x_major_grid.cells()[5] == 29);
    assert(x_major_grid.contains(2, 1) && !x_major_grid.contains(3, 1));

    const DistrictRoom safehouse{1, DistrictRoomType::safehouse_insertion, 2, 2, 6, 6};
    const DistrictRoom nearby_room{2, DistrictRoomType::security_command_hub, 9, 3, 5, 5};
    assert(safehouse.center_x == 5 && safehouse.center_z == 5);
    assert(safehouse.intersects(nearby_room, 2));
    assert(!safehouse.intersects(nearby_room, 0));
    assert(district_room_type_info(DistrictRoomType::megaspire_sanctum).base_security == 5);

    const DoorObject locked_door{3, 4, 5, std::optional<ItemType>{ItemType::keycard_red}};
    const DoorObject open_door{4, 6, 7, std::nullopt, true};
    assert(locked_door.is_locked && !locked_door.is_open);
    assert(open_door.is_open && !open_door.is_locked);

    GeneratedWorld generated{};
    generated.width = 3;
    generated.height = 2;
    generated.grid.reset(3, 2, TileType::wall_concrete);
    generated.heights.reset(3, 2, 2.5F);
    generated.colors.reset(3, 2, 0xFF142436U);
    assert(generated.valid());
    generated.grid.reset(2, 2, TileType::floor);
    assert(!generated.valid());

    return 0;
}
