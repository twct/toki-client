#pragma once

#include <toolkit/color.h>
#include <toolkit/ui/geometry.h>

#include <variant>

namespace toolkit {

struct DrawRectCommand {
    Point origin;
    Size size;
    Color color;
    float corner_radius;
    bool anti_aliasing;
};

using DrawCommand = std::variant<DrawRectCommand>;

}  // namespace toolkit