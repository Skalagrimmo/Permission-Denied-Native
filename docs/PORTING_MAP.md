# Kotlin to Native Porting Map

Source project: C:\Users\scs\Desktop\111\Permission-Denied-lab

The original Android project is a reference implementation. Do not delete or rewrite it while migrating.

| Existing Kotlin source | Native destination | Porting notes |
|---|---|---|
| model/GameModels.kt | core/include/pd/core/ | Convert enums and data classes to strongly typed C++ models. |
| engine/Vector3.kt | core/include/pd/core/math.hpp | Introduce shared vector and angle helpers. |
| engine/PlayerMovementController.kt | core/src/movement.cpp | Preserve normalized input, swept grid collision and axis sliding. |
| engine/WorldGenerator.kt | core/src/world_generator.cpp | Preserve seeded output and district layouts; add deterministic tests. |
| engine/GameEngine.kt | core/src/game.cpp and focused systems | Split the 896-line manager into combat, AI, hacking, mission and interaction systems. |
| model/HackingGraph.kt | core/src/hacking.cpp | Preserve the node graph and hacking session rules. |
| renderer/CyberpunkGlRenderer.kt | renderer/src/ | One native renderer for desktop and Android. |
| renderer/ProceduralLevelMeshGenerator.kt | renderer/src/mesh_generator.cpp | Move geometry generation out of the Kotlin GL thread. |
| renderer/AsciiRasterizer.kt and ASCII shaders | renderer/shaders/ | Recreate ASCII as a post-process, not as a second renderer. |
| audio/GameAudioEngine.kt | core/src/audio/ | Move procedural synthesis to a native audio callback; avoid allocations/locks in callback. |
| data/AppDatabase.kt and GameRepository.kt | Native save layer plus Android storage adapter | Keep save format platform-neutral; SQLite can remain the storage format. |
| Compose screens and MainActivity.kt | android/ | Kotlin should own Activity lifecycle, permissions and platform services only. |

## Migration rules

1. Preserve a clean copy of the Kotlin implementation until parity is verified.
2. Port deterministic systems before presentation systems.
3. Add native tests before replacing each Kotlin subsystem.
4. Keep the simulation independent of SDL and OpenGL.
5. Keep platform APIs outside core/.
6. Never make the Android wrapper a second game engine.
