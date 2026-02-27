#pragma once

#include "geometry.h"

namespace toolkit {

enum class UiEventType { Click, MouseDown, MouseEnter, MouseHover, MouseLeave };

class UiNode;

struct EventBase {
    UiEventType type;

    EventBase(UiEventType type) : type(type) {}
};

struct UiClickEvent: EventBase {
    UiNode& target;
    Point position;

    UiClickEvent(UiNode& target, const Point& position) :
        EventBase(UiEventType::Click),
        target(target),
        position(position) {}
};

struct UiMouseDownEvent: EventBase {
    UiNode& target;
    Point position;

    UiMouseDownEvent(UiNode& target, const Point& position) :
        EventBase(UiEventType::MouseDown),
        target(target),
        position(position) {}
};

struct UiMouseEnterEvent: EventBase {
    UiNode& target;
    Point position;

    UiMouseEnterEvent(UiNode& target, const Point& position) :
        EventBase(UiEventType::MouseEnter),
        target(target),
        position(position) {}
};

struct UiMouseHoverEvent: EventBase {
    UiNode& target;
    Point position;

    UiMouseHoverEvent(UiNode& target, const Point& position) :
        EventBase(UiEventType::MouseHover),
        target(target),
        position(position) {}
};

struct UiMouseLeaveEvent: EventBase {
    UiNode& target;
    Point position;

    UiMouseLeaveEvent(UiNode& target, const Point& position) :
        EventBase(UiEventType::MouseLeave),
        target(target),
        position(position) {}
};

}  // namespace toolkit