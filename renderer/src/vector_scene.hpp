#pragma once

#include "pd/core/game.hpp"
#include "pd/renderer/opengl_renderer.hpp"

#include <vector>

namespace pd::renderer::detail {

struct VectorVertex {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
};

[[nodiscard]] std::vector<VectorVertex> build_vector_scene(
    const pd::core::GameState& state,
    int width,
    int height,
    const TouchHudState& touch);

} // namespace pd::renderer::detail
