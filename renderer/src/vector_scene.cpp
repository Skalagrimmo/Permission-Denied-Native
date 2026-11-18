#include "vector_scene.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <string_view>
#include <utility>

namespace pd::renderer::detail {
namespace {

constexpr float kPi = 3.14159265358979323846F;
constexpr float kNearPlane = 0.10F;
constexpr float kFarPlane = 90.0F;

struct Vec3 {
    float x;
    float y;
    float z;
};

struct CameraPoint {
    float x;
    float y;
    float z;
};

struct Color {
    float r;
    float g;
    float b;
};

class SceneBuilder final {
public:
    SceneBuilder(const pd::core::GameState& state, int width, int height)
        : camera_x_(state.player.x),
          camera_y_(1.62F),
          camera_z_(state.player.z),
          cos_yaw_(std::cos(state.player.yaw_radians)),
          sin_yaw_(std::sin(state.player.yaw_radians)),
          cos_pitch_(std::cos(state.player.pitch_radians)),
          sin_pitch_(std::sin(state.player.pitch_radians)),
          width_(static_cast<float>(std::max(width, 1))),
          height_(static_cast<float>(std::max(height, 1))),
          aspect_(width_ / height_),
          focal_pixels_(height_ * 0.82F) {
        vertices_.reserve(18000);
    }

    std::vector<VectorVertex> build(const TouchHudState& touch) {
        add_floor();
        add_corridor();
        add_columns();
        float first_portal_z = std::floor((camera_z_ - 18.0F) / 48.0F) * 48.0F + 18.0F;
        if (first_portal_z < camera_z_ - 12.0F) first_portal_z += 48.0F;
        add_portal(first_portal_z, "PERMISSION DENIED", "ACCESS REVOKED / 07");
        add_portal(first_portal_z + 48.0F, "SECTOR 08", "NO OPERATOR / NO SIGNAL");
        add_terminals();
        if (touch.visible) add_touch_hud(touch);
        return std::move(vertices_);
    }

private:
    [[nodiscard]] CameraPoint to_camera(const Vec3& point) const noexcept {
        const float tx = point.x - camera_x_;
        const float ty = point.y - camera_y_;
        const float tz = point.z - camera_z_;

        const float x1 = tx * cos_yaw_ - tz * sin_yaw_;
        const float z1 = tx * sin_yaw_ + tz * cos_yaw_;
        const float y2 = ty * cos_pitch_ - z1 * sin_pitch_;
        const float z2 = ty * sin_pitch_ + z1 * cos_pitch_;
        return {x1, y2, z2};
    }

    [[nodiscard]] bool project(const CameraPoint& point, VectorVertex& out, const Color& color) const noexcept {
        if (point.z < kNearPlane || point.z > kFarPlane) return false;
        const float inv_z = 1.0F / point.z;
        const float focal_x = focal_pixels_ * 2.0F / width_;
        const float focal_y = focal_pixels_ * 2.0F / height_;
        const float ndc_x = point.x * inv_z * focal_x;
        const float ndc_y = point.y * inv_z * focal_y;
        if (std::abs(ndc_x) > 5.0F || std::abs(ndc_y) > 5.0F) return false;

        const float near_far = (kFarPlane + kNearPlane) / (kFarPlane - kNearPlane);
        const float depth_term = (2.0F * kFarPlane * kNearPlane) /
            ((kFarPlane - kNearPlane) * point.z);
        out = {
            ndc_x,
            ndc_y,
            near_far - depth_term,
            color.r,
            color.g,
            color.b
        };
        return true;
    }

    void add_line(const Vec3& world_a, const Vec3& world_b, const Color& color) {
        CameraPoint a = to_camera(world_a);
        CameraPoint b = to_camera(world_b);
        if (a.z < kNearPlane && b.z < kNearPlane) return;
        if (a.z > kFarPlane && b.z > kFarPlane) return;

        if (a.z < kNearPlane) {
            const float t = (kNearPlane - a.z) / (b.z - a.z);
            a.x += (b.x - a.x) * t;
            a.y += (b.y - a.y) * t;
            a.z = kNearPlane;
        }
        if (b.z < kNearPlane) {
            const float t = (kNearPlane - b.z) / (a.z - b.z);
            b.x += (a.x - b.x) * t;
            b.y += (a.y - b.y) * t;
            b.z = kNearPlane;
        }
        if (a.z > kFarPlane) {
            const float t = (kFarPlane - a.z) / (b.z - a.z);
            a.x += (b.x - a.x) * t;
            a.y += (b.y - a.y) * t;
            a.z = kFarPlane;
        }
        if (b.z > kFarPlane) {
            const float t = (kFarPlane - b.z) / (a.z - b.z);
            b.x += (a.x - b.x) * t;
            b.y += (a.y - b.y) * t;
            b.z = kFarPlane;
        }

        VectorVertex va{};
        VectorVertex vb{};
        if (project(a, va, color) && project(b, vb, color)) {
            vertices_.push_back(va);
            vertices_.push_back(vb);
        }
    }

    void add_box(
        float min_x, float min_y, float min_z,
        float max_x, float max_y, float max_z,
        const Color& color) {
        const Vec3 p[8] = {
            {min_x, min_y, min_z}, {max_x, min_y, min_z},
            {max_x, max_y, min_z}, {min_x, max_y, min_z},
            {min_x, min_y, max_z}, {max_x, min_y, max_z},
            {max_x, max_y, max_z}, {min_x, max_y, max_z}
        };
        constexpr int edges[12][2] = {
            {0,1},{1,2},{2,3},{3,0},
            {4,5},{5,6},{6,7},{7,4},
            {0,4},{1,5},{2,6},{3,7}
        };
        for (const auto& edge : edges) add_line(p[edge[0]], p[edge[1]], color);
    }

    void add_floor() {
        const float center = 2.5F;
        const float left = center - 10.0F;
        const float right = center + 10.0F;
        const float start_z = std::floor(camera_z_ / 4.0F) * 4.0F - 16.0F;
        const float end_z = camera_z_ + 48.0F;

        const Color fine{0.035F, 0.25F, 0.30F};
        const Color major{0.08F, 0.58F, 0.64F};
        const Color rail{0.08F, 0.82F, 0.82F};

        for (int x = -10; x <= 10; ++x) {
            const float wx = center + static_cast<float>(x);
            const Color& c = (x % 4 == 0) ? major : fine;
            add_line({wx, 0.015F, start_z}, {wx, 0.015F, end_z}, c);
        }
        const int first_z = static_cast<int>(std::floor(start_z));
        const int last_z = static_cast<int>(std::ceil(end_z));
        for (int z = first_z; z <= last_z; ++z) {
            const Color& c = (z % 4 == 0) ? major : fine;
            add_line({left, 0.015F, static_cast<float>(z)},
                     {right, 0.015F, static_cast<float>(z)}, c);
        }

        add_line({center - 6.0F, 0.035F, start_z}, {center - 6.0F, 0.035F, end_z}, rail);
        add_line({center + 6.0F, 0.035F, start_z}, {center + 6.0F, 0.035F, end_z}, rail);
        add_line({center - 1.2F, 0.025F, start_z}, {center - 1.2F, 0.025F, end_z}, fine);
        add_line({center + 1.2F, 0.025F, start_z}, {center + 1.2F, 0.025F, end_z}, fine);
    }

    void add_corridor() {
        constexpr float center = 2.5F;
        constexpr float left = center - 7.0F;
        constexpr float right = center + 7.0F;
        const float start_z = std::floor(camera_z_ / 4.0F) * 4.0F - 16.0F;
        const float end_z = camera_z_ + 48.0F;

        const Color wall{0.035F, 0.36F, 0.42F};
        const Color frame{0.07F, 0.74F, 0.78F};
        const Color brace{0.035F, 0.46F, 0.52F};

        for (float z = start_z; z <= end_z; z += 4.0F) {
            for (const float x : {left, right}) {
                add_line({x, 0.05F, z}, {x, 5.2F, z}, frame);
                add_line({x, 0.05F, z}, {x, 0.05F, z + 4.0F}, wall);
                add_line({x, 2.55F, z}, {x, 2.55F, z + 4.0F}, wall);
                add_line({x, 5.2F, z}, {x, 5.2F, z + 4.0F}, frame);
                add_line({x, 0.1F, z}, {x, 2.5F, z + 4.0F}, brace);
                add_line({x, 2.5F, z + 4.0F}, {x, 5.1F, z}, brace);
            }
        }

        add_line({left, 5.2F, start_z}, {left, 5.2F, end_z}, frame);
        add_line({right, 5.2F, start_z}, {right, 5.2F, end_z}, frame);
        add_line({left, 0.05F, start_z}, {left, 0.05F, end_z}, wall);
        add_line({right, 0.05F, start_z}, {right, 0.05F, end_z}, wall);

        // Overhead cable trays and suspended vector conduits.
        add_line({center - 2.2F, 5.0F, start_z}, {center - 2.2F, 5.0F, end_z}, brace);
        add_line({center + 2.2F, 5.0F, start_z}, {center + 2.2F, 5.0F, end_z}, brace);
        for (float z = start_z + 2.0F; z < end_z; z += 8.0F) {
            add_line({center - 2.2F, 5.0F, z}, {center + 2.2F, 5.0F, z}, wall);
        }
    }

    void add_columns() {
        constexpr float center = 2.5F;
        const Color edge{0.08F, 0.64F, 0.70F};
        const Color detail{0.035F, 0.34F, 0.40F};
        const float first_z = std::floor(camera_z_ / 8.0F) * 8.0F - 8.0F;
        for (float z = first_z; z < camera_z_ + 44.0F; z += 8.0F) {
            for (const float x : {center - 5.9F, center + 5.9F}) {
                add_box(x - 0.28F, 0.05F, z - 0.28F,
                        x + 0.28F, 4.6F, z + 0.28F, edge);
                add_box(x - 0.38F, 0.05F, z - 0.38F,
                        x + 0.38F, 0.40F, z + 0.38F, detail);
                add_line({x, 4.6F, z}, {x, 5.1F, z}, detail);
            }
        }
    }

    void add_portal(float z, std::string_view title, std::string_view subtitle) {
        constexpr float center = 2.5F;
        const Color frame{0.12F, 0.86F, 0.88F};
        const Color panel{0.06F, 0.48F, 0.56F};
        const Color amber{0.96F, 0.52F, 0.20F};

        // Large open gateway: nested frames make depth legible without lighting.
        add_line({center - 4.5F, 0.05F, z}, {center - 4.5F, 4.9F, z}, frame);
        add_line({center + 4.5F, 0.05F, z}, {center + 4.5F, 4.9F, z}, frame);
        add_line({center - 4.5F, 4.9F, z}, {center + 4.5F, 4.9F, z}, frame);
        add_line({center - 3.9F, 0.05F, z + 0.12F}, {center - 3.9F, 4.25F, z + 0.12F}, panel);
        add_line({center + 3.9F, 0.05F, z + 0.12F}, {center + 3.9F, 4.25F, z + 0.12F}, panel);
        add_line({center - 3.9F, 4.25F, z + 0.12F}, {center + 3.9F, 4.25F, z + 0.12F}, panel);
        add_line({center - 4.5F, 2.8F, z}, {center + 4.5F, 2.8F, z}, panel);

        // Header panel outline. Text sits just in front of the panel plane.
        add_line({center - 4.5F, 2.95F, z - 0.035F}, {center + 4.5F, 2.95F, z - 0.035F}, frame);
        add_line({center - 4.5F, 4.75F, z - 0.035F}, {center + 4.5F, 4.75F, z - 0.035F}, frame);
        add_line({center - 4.5F, 2.95F, z - 0.035F}, {center - 4.5F, 4.75F, z - 0.035F}, frame);
        add_line({center + 4.5F, 2.95F, z - 0.035F}, {center + 4.5F, 4.75F, z - 0.035F}, frame);

        add_ascii_text(title, center, 3.82F, z - 0.06F, 0.062F, frame);
        add_ascii_text(subtitle, center, 3.22F, z - 0.06F, 0.036F, amber);

        // Threshold markers and short floor chevrons.
        for (int i = -3; i <= 3; ++i) {
            const float x = center + static_cast<float>(i) * 1.0F;
            add_line({x, 0.06F, z - 0.9F}, {x, 0.06F, z - 0.35F}, panel);
        }
        add_line({center - 0.6F, 0.07F, z - 1.2F}, {center, 0.07F, z - 0.65F}, amber);
        add_line({center + 0.6F, 0.07F, z - 1.2F}, {center, 0.07F, z - 0.65F}, amber);
    }

    void add_terminals() {
        constexpr float center = 2.5F;
        const Color casing{0.06F, 0.59F, 0.66F};
        const Color screen{0.12F, 0.82F, 0.76F};
        const Color amber{0.95F, 0.47F, 0.17F};

        const float first_z = std::floor(camera_z_ / 16.0F) * 16.0F - 16.0F;
        for (int i = 0; i < 3; ++i) {
            const float z = first_z + static_cast<float>(i) * 16.0F;
            for (const int side : {-1, 1}) {
                const float x = center + static_cast<float>(side) * 5.95F;
                add_box(x - 0.52F, 0.15F, z - 0.45F,
                        x + 0.52F, 2.65F, z + 0.45F, casing);
                // Forward-facing display inset on the console front.
                const float front_z = z - 0.47F;
                add_line({x - 0.38F, 1.2F, front_z}, {x + 0.38F, 1.2F, front_z}, screen);
                add_line({x - 0.38F, 2.35F, front_z}, {x + 0.38F, 2.35F, front_z}, screen);
                add_line({x - 0.38F, 1.2F, front_z}, {x - 0.38F, 2.35F, front_z}, screen);
                add_line({x + 0.38F, 1.2F, front_z}, {x + 0.38F, 2.35F, front_z}, screen);
                for (int row = 0; row < 4; ++row) {
                    const float y = 1.42F + static_cast<float>(row) * 0.19F;
                    add_line({x - 0.27F, y, front_z - 0.01F},
                             {x + 0.12F + static_cast<float>(row % 2) * 0.12F, y, front_z - 0.01F},
                             row == 0 ? amber : screen);
                }
            }
        }
    }

    static std::array<std::uint8_t, 7> glyph_for(char ch) noexcept {
        constexpr std::string_view alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        // Five-column, seven-row bitmap glyphs; 0x10 is the leftmost pixel.
        static constexpr std::array<std::array<std::uint8_t, 7>, 36> font{{
            {{14,17,19,21,25,17,14}}, {{4,12,4,4,4,4,14}},
            {{14,17,1,2,4,8,31}}, {{30,1,1,14,1,1,30}},
            {{2,6,10,18,31,2,2}}, {{31,16,16,30,1,1,30}},
            {{14,16,16,30,17,17,14}}, {{31,1,2,4,8,8,8}},
            {{14,17,17,14,17,17,14}}, {{14,17,17,15,1,1,14}},
            {{14,17,17,31,17,17,17}}, {{30,17,17,30,17,17,30}},
            {{14,17,16,16,16,17,14}}, {{30,17,17,17,17,17,30}},
            {{31,16,16,30,16,16,31}}, {{31,16,16,30,16,16,16}},
            {{14,17,16,23,17,17,15}}, {{17,17,17,31,17,17,17}},
            {{14,4,4,4,4,4,14}}, {{7,2,2,2,18,18,12}},
            {{17,18,20,24,20,18,17}}, {{16,16,16,16,16,16,31}},
            {{17,27,21,21,17,17,17}}, {{17,25,21,19,17,17,17}},
            {{14,17,17,17,17,17,14}}, {{30,17,17,30,16,16,16}},
            {{14,17,17,17,21,18,13}}, {{30,17,17,30,20,18,17}},
            {{15,16,16,14,1,1,30}}, {{31,4,4,4,4,4,4}},
            {{17,17,17,17,17,17,14}}, {{17,17,17,17,17,10,4}},
            {{17,17,17,21,21,21,10}}, {{17,17,10,4,10,17,17}},
            {{17,17,10,4,4,4,4}}, {{31,1,2,4,8,16,31}}
        }};

        const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        const auto pos = alphabet.find(upper);
        if (pos != std::string_view::npos) return font[pos];
        switch (upper) {
            case '-': return {0,0,0,31,0,0,0};
            case '/': return {1,1,2,4,8,16,16};
            case ':': return {0,4,4,0,4,4,0};
            case '.': return {0,0,0,0,0,12,12};
            case '[': return {14,8,8,8,8,8,14};
            case ']': return {14,2,2,2,2,2,14};
            case '>': return {16,8,4,2,4,8,16};
            case '<': return {1,2,4,8,4,2,1};
            case '_': return {0,0,0,0,0,0,31};
            default: return {0,0,0,0,0,0,0};
        }
    }

    void add_ascii_text(
        std::string_view text,
        float center_x,
        float bottom_y,
        float z,
        float cell,
        const Color& color) {
        if (text.empty()) return;
        const float advance = cell * 6.0F;
        const float full_width = advance * static_cast<float>(text.size()) - cell;
        const float start_x = center_x - full_width * 0.5F;

        for (std::size_t index = 0; index < text.size(); ++index) {
            const auto glyph = glyph_for(text[index]);
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    const std::uint8_t mask = static_cast<std::uint8_t>(1U << (4 - col));
                    if ((glyph[static_cast<std::size_t>(row)] & mask) == 0) continue;

                    const float x0 = start_x + (static_cast<float>(index) * 6.0F + static_cast<float>(col)) * cell;
                    const float y1 = bottom_y + static_cast<float>(6 - row) * cell;
                    const float x1 = x0 + cell * 0.88F;
                    const float y0 = y1 - cell * 0.88F;
                    const float face_z = z - 0.002F;
                    add_line({x0,y0,face_z},{x1,y0,face_z},color);
                    add_line({x1,y0,face_z},{x1,y1,face_z},color);
                    add_line({x1,y1,face_z},{x0,y1,face_z},color);
                    add_line({x0,y1,face_z},{x0,y0,face_z},color);
                }
            }
        }
    }

    void add_screen_line(float u0, float v0, float u1, float v1, const Color& color) {
        vertices_.push_back({u0 * 2.0F - 1.0F, v0 * 2.0F - 1.0F, -0.985F, color.r, color.g, color.b});
        vertices_.push_back({u1 * 2.0F - 1.0F, v1 * 2.0F - 1.0F, -0.985F, color.r, color.g, color.b});
    }

    void add_screen_circle(float cx, float cy, float radius, const Color& color) {
        constexpr int segments = 36;
        const float inv_aspect = 1.0F / std::max(aspect_, 1.0F);
        for (int i = 0; i < segments; ++i) {
            const float a0 = (static_cast<float>(i) / segments) * 2.0F * kPi;
            const float a1 = (static_cast<float>(i + 1) / segments) * 2.0F * kPi;
            add_screen_line(
                cx + std::cos(a0) * radius * inv_aspect, cy + std::sin(a0) * radius,
                cx + std::cos(a1) * radius * inv_aspect, cy + std::sin(a1) * radius,
                color);
        }
    }

    void add_touch_hud(const TouchHudState& touch) {
        const Color cyan{0.08F, 0.78F, 0.88F};
        const Color bright{0.28F, 0.96F, 0.98F};
        const Color muted{0.04F, 0.42F, 0.52F};
        const float stick_x = 0.16F + touch.move_x * 0.066F / std::max(aspect_, 1.0F);
        const float stick_y = 0.21F + touch.move_y * 0.066F;

        add_screen_circle(0.16F, 0.21F, 0.132F, muted);
        add_screen_circle(0.16F, 0.21F, 0.112F, cyan);
        add_screen_circle(stick_x, stick_y, 0.044F, touch.move_active ? bright : cyan);
        add_screen_line(0.16F - 0.018F / aspect_, 0.21F, 0.16F + 0.018F / aspect_, 0.21F, muted);
        add_screen_line(0.16F, 0.192F, 0.16F, 0.228F, muted);

        const Color sprint_color = touch.sprint_active ? bright : cyan;
        add_screen_circle(0.88F, 0.21F, 0.154F, muted);
        add_screen_circle(0.88F, 0.21F, 0.136F, sprint_color);
        add_screen_line(0.88F, 0.184F, 0.88F, 0.236F, sprint_color);
        add_screen_line(0.868F, 0.210F, 0.892F, 0.210F, sprint_color);
    }

    float camera_x_;
    float camera_y_;
    float camera_z_;
    float cos_yaw_;
    float sin_yaw_;
    float cos_pitch_;
    float sin_pitch_;
    float width_;
    float height_;
    float aspect_;
    float focal_pixels_;
    std::vector<VectorVertex> vertices_;
};

} // namespace

std::vector<VectorVertex> build_vector_scene(
    const pd::core::GameState& state,
    int width,
    int height,
    const TouchHudState& touch) {
    SceneBuilder builder(state, width, height);
    return builder.build(touch);
}

} // namespace pd::renderer::detail
