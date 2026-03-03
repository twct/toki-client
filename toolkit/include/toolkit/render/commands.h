#pragma once

#include <toolkit/color.h>
#include <toolkit/ui/geometry.h>

#include <cstdint>
#include <variant>

namespace toolkit {

struct BorderWidths {
    float left = 0.f;
    float top = 0.f;
    float right = 0.f;
    float bottom = 0.f;

    bool has_border() const {
        return left > 0.f || top > 0.f || right > 0.f || bottom > 0.f;
    }

    bool is_uniform() const {
        return left == top && top == right && right == bottom;
    }
};

class BoxShadow {
  public:
    BoxShadow() = default;

    BoxShadow(
        float offset_x,
        float offset_y,
        float blur_radius,
        float spread_radius,
        const Color& color
    ) :
        m_offset_x(offset_x),
        m_offset_y(offset_y),
        m_blur_radius(blur_radius),
        m_spread_radius(spread_radius),
        m_color(color) {}

    BoxShadow with_offset_x(float offset_x) const {
        BoxShadow s = *this;
        s.m_offset_x = offset_x;
        return s;
    }

    BoxShadow with_offset_y(float offset_y) const {
        BoxShadow s = *this;
        s.m_offset_y = offset_y;
        return s;
    }

    BoxShadow with_offset(float x, float y) const {
        BoxShadow s = *this;
        s.m_offset_x = x;
        s.m_offset_y = y;
        return s;
    }

    BoxShadow with_blur(float blur_radius) const {
        BoxShadow s = *this;
        s.m_blur_radius = blur_radius;
        return s;
    }

    BoxShadow with_spread(float spread_radius) const {
        BoxShadow s = *this;
        s.m_spread_radius = spread_radius;
        return s;
    }

    BoxShadow with_color(const Color& color) const {
        BoxShadow s = *this;
        s.m_color = color;
        return s;
    }

    bool has_shadow() const {
        return m_color.a > 0.f
            && (m_blur_radius > 0.f || m_spread_radius > 0.f
                || m_offset_x != 0.f || m_offset_y != 0.f);
    }

    float offset_x() const { return m_offset_x; }
    float offset_y() const { return m_offset_y; }
    float blur_radius() const { return m_blur_radius; }
    float spread_radius() const { return m_spread_radius; }
    const Color& color() const { return m_color; }

  private:
    float m_offset_x = 0.f;
    float m_offset_y = 0.f;
    float m_blur_radius = 0.f;
    float m_spread_radius = 0.f;
    Color m_color {0.f, 0.f, 0.f, 0.f};
};

struct DrawRectCommand {
    Point origin;
    Size size;
    Color color;
    float corner_radius;
    bool anti_aliasing;
    Color border_color;
    BorderWidths border;
    BoxShadow shadow;
};

enum class ImageFit {
    Fill,    // stretch to fill, ignoring aspect ratio
    Contain, // scale to fit within bounds, letterboxed
    Cover,   // scale to fill bounds, cropping overflow
    None,    // original size, centered, cropped if larger than bounds
};

struct DrawImageCommand {
    size_t image_id;
    Point origin;
    Size size;
    ImageFit fit = ImageFit::Fill;
};

struct ClipRRectCommand {
    Point origin;
    Size size;
    float corner_radius;
};

struct ClipRestoreCommand {};

using DrawCommand = std::variant<DrawRectCommand, DrawImageCommand, ClipRRectCommand, ClipRestoreCommand>;

}  // namespace toolkit