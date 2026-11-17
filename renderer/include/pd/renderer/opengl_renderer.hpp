#pragma once
#include "pd/core/game.hpp"
#include <memory>

namespace pd::renderer {
class OpenGLRenderer final {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();
    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;
    [[nodiscard]] bool initialize();
    void resize(int width, int height) noexcept;
    void render(const pd::core::GameState& state, float time_seconds) noexcept;
    void shutdown() noexcept;
    [[nodiscard]] bool is_initialized() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace pd::renderer
