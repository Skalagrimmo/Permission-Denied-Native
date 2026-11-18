#pragma once
#include "pd/core/game.hpp"
#include <memory>

namespace pd::renderer {
struct TouchHudState {
    float move_x = 0.0F;
    float move_y = 0.0F;
    bool move_active = false;
    bool sprint_active = false;
    bool visible = false;
};

class OpenGLRenderer final {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();
    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;
    [[nodiscard]] bool initialize();
    void resize(int width, int height) noexcept;
    void render(
        const pd::core::GameState& state,
        float time_seconds,
        const TouchHudState& touch = {}) noexcept;
    void shutdown() noexcept;
    [[nodiscard]] bool is_initialized() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace pd::renderer
