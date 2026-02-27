#pragma once

#include <toolkit/color.h>
#include <toolkit/render/commands.h>

#include <variant>
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
    void draw_rect(
        const Point& origin,
        const Size& size,
        const Color& color,
        float corner_radius = 0.f,
        AntiAliasing anti_aliasing = AntiAliasing::Enabled,
        const Color& border_color = {0.f, 0.f, 0.f, 0.f},
        const BorderWidths& border = {}
    ) {
        m_commands.push_back(
            DrawRectCommand {
                .origin = origin,
                .size = size,
                .color = color,
                .corner_radius = corner_radius,
                .anti_aliasing = to_bool(anti_aliasing),
                .border_color = border_color,
                .border = border
            }
        );
    }

    const std::vector<DrawCommand>& commands() const {
        return m_commands;
    }

    void clear() {
        m_commands.clear();
    }

  private:
    std::vector<DrawCommand> m_commands;
};

}  // namespace toolkit