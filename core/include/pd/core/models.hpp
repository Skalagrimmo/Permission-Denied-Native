#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace pd::core {

enum class WeaponType {
    silenced_pistol,
    arc_stunner,
    cyber_smg,
    emp_grenade,
    cyberdeck,
    lockpick
};

struct WeaponTypeInfo {
    std::string_view display_name;
    bool is_lethal;
    float base_noise_radius;
};

[[nodiscard]] constexpr WeaponTypeInfo weapon_type_info(WeaponType type) noexcept {
    switch (type) {
    case WeaponType::silenced_pistol: return {"Silenced Whisper", true, 3.5F};
    case WeaponType::arc_stunner: return {"Arc Stunner", false, 1.0F};
    case WeaponType::cyber_smg: return {"Spectre SMG", true, 18.0F};
    case WeaponType::emp_grenade: return {"EMP Disruptor", false, 12.0F};
    case WeaponType::cyberdeck: return {"Cyberdeck MK-IV", false, 0.0F};
    case WeaponType::lockpick: return {"Nanowire Pick", false, 0.5F};
    }
    return {};
}

struct Weapon final {
    WeaponType type = WeaponType::silenced_pistol;
    int ammo_in_mag = 0;
    int max_mag = 0;
    int reserve_ammo = 0;
    int damage = 0;
    std::int64_t fire_rate_ms = 0;
    std::int64_t reload_time_ms = 0;
    std::int64_t last_fired_time = 0;
    bool is_reloading = false;
    std::int64_t reload_start_time = 0;

    [[nodiscard]] bool can_fire() const noexcept {
        return ammo_in_mag > 0 && !is_reloading;
    }

    [[nodiscard]] static Weapon create_default_pistol() noexcept {
        return {WeaponType::silenced_pistol, 12, 12, 36, 38, 280, 1200};
    }
    [[nodiscard]] static Weapon create_stunner() noexcept {
        return {WeaponType::arc_stunner, 1, 1, 8, 100, 1000, 1500};
    }
    [[nodiscard]] static Weapon create_smg() noexcept {
        return {WeaponType::cyber_smg, 30, 30, 90, 22, 95, 1800};
    }
    [[nodiscard]] static Weapon create_emp_grenade() noexcept {
        return {WeaponType::emp_grenade, 3, 3, 3, 60, 800, 500};
    }
    [[nodiscard]] static Weapon create_cyberdeck() noexcept {
        return {WeaponType::cyberdeck, 100, 100, 100, 0, 500, 0};
    }
    [[nodiscard]] static Weapon create_lockpick() noexcept {
        return {WeaponType::lockpick, 5, 5, 5, 0, 500, 0};
    }
};

enum class AugmentationType {
    active_camo,
    cyber_optics,
    dash_thrusters,
    neural_overclock,
    subdermal_plating,
    audio_dampener
};

struct AugmentationInfo {
    std::string_view title;
    std::string_view subtitle;
    std::string_view description;
    float energy_cost_per_sec;
    float energy_cost_instant;
    std::int64_t cooldown_ms;
};

[[nodiscard]] constexpr AugmentationInfo augmentation_info(AugmentationType type) noexcept {
    switch (type) {
    case AugmentationType::active_camo:
        return {"Ghost Camouflage", "Optical Cloak",
                "Refracts light to reduce visual detection by 90%. Consumes 12 energy/sec.",
                12.0F, 0.0F, 2000};
    case AugmentationType::cyber_optics:
        return {"Cybernetic Optics", "Thermal & Security HUD",
                "Highlights enemies, cameras, laser grids, and loot through walls.",
                6.0F, 0.0F, 1000};
    case AugmentationType::dash_thrusters:
        return {"Kinetic Thrusters", "Rapid Dash",
                "Propels player forward instantly in a burst of speed. Clears laser grids.",
                0.0F, 25.0F, 2500};
    case AugmentationType::neural_overclock:
        return {"Neural Overclock", "Reflex Dilation",
                "Dilates perception by 50% for 4.5 seconds for precision aiming and evasion.",
                0.0F, 40.0F, 8000};
    case AugmentationType::subdermal_plating:
        return {"Subdermal Plating", "Ballistic Armor",
                "Passive bio-weave reduces incoming kinetic and shock damage by 35%.",
                0.0F, 0.0F, 0};
    case AugmentationType::audio_dampener:
        return {"Acoustic Dampener", "Silent Steps",
                "Passive leg dampers eliminate footstep audio and reduce sprint noise by 80%.",
                0.0F, 0.0F, 0};
    }
    return {};
}

enum class ItemType {
    health_stim,
    armor_plate,
    energy_battery,
    emp_cell,
    keycard_blue,
    keycard_red,
    evidence_slate,
    faction_contract
};

struct InventoryItem final {
    std::string id;
    ItemType type = ItemType::health_stim;
    std::string name;
    std::string description;
    int count = 1;
};

enum class EnemyType {
    security_guard,
    heavy_enforcer,
    security_drone,
    surveillance_camera,
    ceiling_turret,
    aegis_paladin
};

struct EnemyTypeInfo {
    int max_hp;
    int max_armor;
    float speed;
    float detection_fov;
};

[[nodiscard]] constexpr EnemyTypeInfo enemy_type_info(EnemyType type) noexcept {
    switch (type) {
    case EnemyType::security_guard: return {60, 0, 2.2F, 75.0F};
    case EnemyType::heavy_enforcer: return {130, 80, 1.8F, 65.0F};
    case EnemyType::security_drone: return {45, 0, 3.4F, 85.0F};
    case EnemyType::surveillance_camera: return {30, 0, 0.0F, 90.0F};
    case EnemyType::ceiling_turret: return {100, 40, 0.0F, 80.0F};
    case EnemyType::aegis_paladin: return {400, 250, 2.6F, 100.0F};
    }
    return {};
}

enum class EnemyAiState {
    idle,
    patrol,
    investigating,
    alert,
    combat,
    unconscious,
    dead,
    hacked_friendly
};

struct PatrolPoint {
    float x = 0.0F;
    float z = 0.0F;
};

struct Enemy final {
    int id = 0;
    EnemyType type = EnemyType::security_guard;
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
    float yaw = 0.0F;
    int hp = 60;
    int armor = 0;
    EnemyAiState state = EnemyAiState::patrol;
    float alert_level = 0.0F;
    float target_x = 0.0F;
    float target_z = 0.0F;
    std::vector<PatrolPoint> patrol_points;
    int current_patrol_index = 0;
    float state_timer = 0.0F;
    float shoot_cooldown = 0.0F;
    bool is_boss = false;
    int boss_phase = 1;
    bool boss_shield_active = false;
    bool is_powered_off = false;

    Enemy() = default;

    Enemy(int enemy_id, EnemyType enemy_type, float pos_x, float pos_y,
          float pos_z, float heading) noexcept
        : id(enemy_id),
          type(enemy_type),
          x(pos_x),
          y(pos_y),
          z(pos_z),
          yaw(heading),
          hp(enemy_type_info(enemy_type).max_hp),
          armor(enemy_type_info(enemy_type).max_armor),
          target_x(pos_x),
          target_z(pos_z) {}
};

enum class TileType {
    empty,
    floor,
    wall_concrete,
    wall_glass,
    wall_corp_panel,
    door_closed,
    door_open,
    door_locked_blue,
    door_locked_red,
    vent_shaft,
    terminal,
    chest_loot,
    laser_tripwire,
    checkpoint_terminal,
    spire_elevator,
    ghost_index_mainframe,
    extraction_zone
};

struct TileTypeInfo {
    bool is_solid;
    bool blocks_sight;
    char glyph;
};

[[nodiscard]] constexpr TileTypeInfo tile_type_info(TileType type) noexcept {
    switch (type) {
    case TileType::empty: return {false, false, ' '};
    case TileType::floor: return {false, false, '.'};
    case TileType::wall_concrete: return {true, true, '#'};
    case TileType::wall_glass: return {true, false, '|'};
    case TileType::wall_corp_panel: return {true, true, 'H'};
    case TileType::door_closed: return {true, true, '+'};
    case TileType::door_open: return {false, false, '/'};
    case TileType::door_locked_blue: return {true, true, 'B'};
    case TileType::door_locked_red: return {true, true, 'R'};
    case TileType::vent_shaft: return {false, false, '='};
    case TileType::terminal: return {true, false, '$'};
    case TileType::chest_loot: return {true, false, '8'};
    case TileType::laser_tripwire: return {false, false, '!'};
    case TileType::checkpoint_terminal: return {true, false, '&'};
    case TileType::spire_elevator: return {false, false, '%'};
    case TileType::ghost_index_mainframe: return {true, false, '@'};
    case TileType::extraction_zone: return {false, false, '*'};
    }
    return {true, true, '?'};
}

struct MapTile final {
    TileType type = TileType::floor;
    float height = 1.0F;
    std::uint32_t color = 0xFF00FF00U;
    std::string metadata;
};

enum class DistrictId {
    district_01,
    district_02,
    district_03,
    district_04,
    district_05
};

struct DistrictInfo {
    int level;
    std::string_view title;
    std::string_view theme;
    int size;
};

[[nodiscard]] constexpr DistrictInfo district_info(DistrictId district) noexcept {
    switch (district) {
    case DistrictId::district_01: return {1, "District 01: Neon Spires", "Infiltration & Outskirts", 8};
    case DistrictId::district_02: return {2, "District 02: Slum Grid", "Drone Alley & Black Market", 10};
    case DistrictId::district_03: return {3, "District 03: Industrial Core", "Heavy Enforcers & Laser Arrays", 12};
    case DistrictId::district_04: return {4, "District 04: Corporate Plaza", "Aegis High-Sec & Biometrics", 14};
    case DistrictId::district_05: return {5, "District 05: Census Megaspire", "The Aegis Paladin & Ghost Index", 16};
    }
    return {};
}

enum class EndingChoice { leak, sell, surrender, destroy };

struct EndingInfo {
    std::string_view title;
    std::string_view description;
    int score_bonus;
};

[[nodiscard]] constexpr EndingInfo ending_info(EndingChoice choice) noexcept {
    switch (choice) {
    case EndingChoice::leak:
        return {"Leak to Public", "Broadcast encrypted citizen records. Sparks an underground uprising.", 5000};
    case EndingChoice::sell:
        return {"Sell to Syndicate", "Trade the Ghost Index on the black market for maximum untraceable credits.", 4000};
    case EndingChoice::surrender:
        return {"Surrender to Megacorp", "Return the Index for corporate immunity and executive clearance.", 2500};
    case EndingChoice::destroy:
        return {"Destroy Forever", "Wipe the database permanently. No one controls the city's ghosts.", 6000};
    }
    return {};
}

struct MissionReport final {
    int district_level = 0;
    std::string district_name;
    int score = 0;
    std::string stealth_rating;
    int alarms_triggered = 0;
    int enemies_killed = 0;
    int enemies_stunned = 0;
    int terminals_hacked = 0;
    int evidence_found = 0;
    std::int64_t time_elapsed_seconds = 0;
    std::optional<EndingChoice> ending_choice;
    bool is_victory = false;
};

} // namespace pd::core
