#pragma once

#include <toolkit/color.h>
#include <toolkit/ui/geometry.h>

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

struct DrawRectCommand {
    Point origin;
    Size size;
    Color color;
    float corner_radius;
    bool anti_aliasing;
    Color border_color;
    BorderWidths border;
};

using DrawCommand = std::variant<DrawRectCommand>;

}  // namespace toolkit