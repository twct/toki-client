#pragma once

#include <toolkit/color.h>
#include <toolkit/render/commands.h>

#include <vector>

#include "geometry.h"

namespace toolkit {

class UiBorder;

enum class AntiAliasing { Enabled, Disabled };

inline bool to_bool(AntiAliasing anti_aliasing) {
    if (anti_aliasing == AntiAliasing::Enabled) {
        return true;
    }

    return false;
}

class Painter {
  public:
    void set_translation(const Point& t) { m_translation = t; }

    void draw_rect(
        const Point& origin,
        const Size& size,
        const Color& color,
        float corner_radius = 0.f,
        AntiAliasing anti_aliasing = AntiAliasing::Enabled,
        const Color& border_color = {0.f, 0.f, 0.f, 0.f},
        const BorderWidths& border = {},
        const BoxShadow& shadow = {}
    ) {
        m_commands.push_back(
            DrawRectCommand {
                .origin = {origin.x + m_translation.x, origin.y + m_translation.y},
                .size = size,
                .color = color,
                .corner_radius = corner_radius,
                .anti_aliasing = to_bool(anti_aliasing),
                .border_color = border_color,
                .border = border,
                .shadow = shadow
            }
        );
    }

    void push_clip(const Point& origin, const Size& size, float corner_radius) {
        m_commands.push_back(ClipRRectCommand {
            .origin = {origin.x + m_translation.x, origin.y + m_translation.y},
            .size = size,
            .corner_radius = corner_radius,
        });
    }

    void pop_clip() {
        m_commands.push_back(ClipRestoreCommand {});
    }

    void draw_image(
        const Point& origin,
        const Size& size,
        size_t image_id,
        ImageFit fit = ImageFit::Fill
    ) {
        m_commands.push_back(DrawImageCommand {
            .image_id = image_id,
            .origin = {origin.x + m_translation.x, origin.y + m_translation.y},
            .size = size,
            .fit = fit,
        });
    }

    const std::vector<DrawCommand>& commands() const {
        return m_commands;
    }

    void clear() {
        m_commands.clear();
    }

  private:
    std::vector<DrawCommand> m_commands;
    Point m_translation {0.f, 0.f};
};

}  // namespace toolkit
