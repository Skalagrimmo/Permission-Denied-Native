# Core Model Parity — 2026-11-18

Sources:
- Permission-Denied-lab/app/src/main/java/com/example/model/GameModels.kt
- Permission-Denied-lab/app/src/main/java/com/example/engine/WorldGenerator.kt (top-level world data classes)

Native headers:
- core/include/pd/core/models.hpp
- core/include/pd/core/world.hpp

## Ported model groups

- Weapons and their metadata/default loadouts.
- Augmentation definitions and energy/cooldown costs.
- Inventory item types and inventory records.
- Enemy types/stats, AI states, patrol points and mutable enemy state.
- Tile solidity, sight-blocking and glyph metadata.
- District definitions, ending choices and mission reports.
- X-major grid storage, district rooms, terminals, loot chests, doors and generated-world metadata.

## Compatibility notes

- Kotlin Long values map to std::int64_t.
- Kotlin nullable keycards and endings map to std::optional.
- Tile grids preserve Kotlin's grid[x][z] orientation through Grid2D::at(x, z).
- Tile ARGB colors keep the same 32-bit bit pattern using std::uint32_t.
- Enemy construction initializes HP and armor from the selected enemy type, matching Kotlin defaults.
- Door lock state defaults to whether a keycard is required, matching the Kotlin constructor default.

This step defines data contracts only. Procedural generation and movement/collision algorithms are ported separately.
