#pragma once

#include "pd/core/models.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace pd::core {

// Kotlin stores each layer as outer[x][z]. This grid keeps that same X-major order
// in a contiguous vector: index(x, z) = x * height + z.
template <typename T>
class Grid2D final {
public:
    Grid2D() = default;

    Grid2D(int width, int height, const T& initial_value = T{}) {
        reset(width, height, initial_value);
    }

    void reset(int width, int height, const T& initial_value = T{}) {
        if (width < 0 || height < 0) {
            throw std::invalid_argument("Grid2D dimensions cannot be negative");
        }
        width_ = width;
        height_ = height;
        cells_.assign(static_cast<std::size_t>(width_) *
                          static_cast<std::size_t>(height_),
                      initial_value);
    }

    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }
    [[nodiscard]] bool empty() const noexcept { return cells_.empty(); }

    [[nodiscard]] bool contains(int x, int z) const noexcept {
        return x >= 0 && z >= 0 && x < width_ && z < height_;
    }

    [[nodiscard]] std::size_t index(int x, int z) const {
        if (!contains(x, z)) {
            throw std::out_of_range("Grid2D coordinate is outside its bounds");
        }
        return static_cast<std::size_t>(x) * static_cast<std::size_t>(height_) +
               static_cast<std::size_t>(z);
    }

    T& at(int x, int z) { return cells_.at(index(x, z)); }
    const T& at(int x, int z) const { return cells_.at(index(x, z)); }

    T& operator()(int x, int z) { return at(x, z); }
    const T& operator()(int x, int z) const { return at(x, z); }

    [[nodiscard]] const std::vector<T>& cells() const noexcept { return cells_; }
    [[nodiscard]] std::vector<T>& cells() noexcept { return cells_; }

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<T> cells_;
};

enum class DistrictRoomType {
    safehouse_insertion,
    security_command_hub,
    server_mainframe_vault,
    cybernetics_lab,
    armory_supply_cache,
    maintenance_vent_grid,
    executive_office,
    patrol_plaza,
    megaspire_sanctum,
    extraction_rooftop
};

struct DistrictRoomTypeInfo {
    const char* display_name;
    int base_security;
};

[[nodiscard]] constexpr DistrictRoomTypeInfo district_room_type_info(
    DistrictRoomType type) noexcept {
    switch (type) {
    case DistrictRoomType::safehouse_insertion: return {"Safehouse Insertion Point", 1};
    case DistrictRoomType::security_command_hub: return {"Security Command Hub", 3};
    case DistrictRoomType::server_mainframe_vault: return {"Server Mainframe Vault", 4};
    case DistrictRoomType::cybernetics_lab: return {"Cybernetics Research Lab", 2};
    case DistrictRoomType::armory_supply_cache: return {"Armory Supply Cache", 3};
    case DistrictRoomType::maintenance_vent_grid: return {"Maintenance & Vent Chasm", 1};
    case DistrictRoomType::executive_office: return {"Executive Corporate Suite", 4};
    case DistrictRoomType::patrol_plaza: return {"Surveillance Atrium", 2};
    case DistrictRoomType::megaspire_sanctum: return {"Megaspire Core Sanctum", 5};
    case DistrictRoomType::extraction_rooftop: return {"Extraction Rooftop Helipad", 1};
    }
    return {"Unknown", 0};
}

struct DistrictRoom final {
    int id = 0;
    DistrictRoomType type = DistrictRoomType::patrol_plaza;
    int x = 0;
    int z = 0;
    int width = 0;
    int height = 0;
    int center_x = 0;
    int center_z = 0;

    DistrictRoom() = default;

    DistrictRoom(int room_id, DistrictRoomType room_type, int room_x, int room_z,
                 int room_width, int room_height) noexcept
        : id(room_id),
          type(room_type),
          x(room_x),
          z(room_z),
          width(room_width),
          height(room_height),
          center_x(room_x + room_width / 2),
          center_z(room_z + room_height / 2) {}

    [[nodiscard]] bool intersects(const DistrictRoom& other, int padding = 1) const noexcept {
        return x - padding < other.x + other.width &&
               x + width + padding > other.x &&
               z - padding < other.z + other.height &&
               z + height + padding > other.z;
    }
};

struct TerminalObject final {
    int id = 0;
    float x = 0.0F;
    float z = 0.0F;
    std::string sector_name;
    int security_ice_level = 1;
    bool is_hacked = false;
    bool controls_turrets = true;
    bool controls_cameras = true;
    bool controls_power = true;
};

struct LootChestObject final {
    int id = 0;
    float x = 0.0F;
    float z = 0.0F;
    ItemType item_type = ItemType::health_stim;
    int count = 1;
    bool is_opened = false;
};

struct DoorObject final {
    int id = 0;
    int x = 0;
    int z = 0;
    std::optional<ItemType> required_keycard;
    bool is_open = false;
    bool is_locked = false;

    DoorObject() = default;

    DoorObject(int door_id, int door_x, int door_z,
               std::optional<ItemType> keycard,
               bool open = false,
               std::optional<bool> locked = std::nullopt) noexcept
        : id(door_id),
          x(door_x),
          z(door_z),
          required_keycard(keycard),
          is_open(open),
          is_locked(locked.value_or(keycard.has_value())) {}
};

struct GeneratedWorld final {
    DistrictId district = DistrictId::district_01;
    std::int64_t seed = 0;
    int width = 0;
    int height = 0;
    Grid2D<TileType> grid;
    Grid2D<float> heights;
    Grid2D<std::uint32_t> colors;
    float spawn_x = 0.0F;
    float spawn_z = 0.0F;
    float extraction_x = 0.0F;
    float extraction_z = 0.0F;
    float spire_core_x = 0.0F;
    float spire_core_z = 0.0F;
    std::vector<Enemy> enemies;
    std::vector<TerminalObject> terminals;
    std::vector<LootChestObject> loot_chests;
    std::vector<DoorObject> doors;
    int evidence_count = 0;
    int contracts_count = 0;
    std::vector<DistrictRoom> rooms;

    [[nodiscard]] bool valid() const noexcept {
        return width > 0 && height > 0 &&
               grid.width() == width && grid.height() == height &&
               heights.width() == width && heights.height() == height &&
               colors.width() == width && colors.height() == height;
    }
};

} // namespace pd::core
