#pragma once

#include <yoga/YGNode.h>

#include <flat_map>
#include <functional>
#include <memory>
#include <typeindex>
#include <utility>
#include <vector>

#include "color.h"
#include "ui/events.h"
#include "ui/geometry.h"
#include "ui/painter.h"
#include "util/function_traits.h"

namespace toolkit {

enum class ValType { Px, Percent };

class Val {
  public:
    static Val Px(float amount) {
        Val val;
        val.m_type = ValType::Px;
        val.m_amount = amount;
        return val;
    }

    static Val Percent(float amount) {
        Val val;
        val.m_type = ValType::Percent;
        val.m_amount = amount;
        return val;
    }

  private:
    friend class UiNode;
    float m_amount {0.f};
    ValType m_type {ValType::Px};
};

template<typename Derived>
class UiBoxValue {
  public:
    UiBoxValue(float left, float top, float right, float bottom) :
        m_left(left),
        m_top(top),
        m_right(right),
        m_bottom(bottom) {}

    static Derived all(float value) {
        return Derived {value, value, value, value};
    }

    static Derived bottom(float bottom_value) {
        return Derived {0.f, 0.f, 0.f, bottom_value};
    }

    static Derived top(float top_value) {
        return Derived {0.f, top_value, 0.f, 0.f};
    }

    static Derived left(float left_value) {
        return Derived {left_value, 0.f, 0.f, 0.f};
    }

    static Derived right(float right_value) {
        return Derived {0.f, 0.f, right_value, 0.f};
    }

    static Derived horizontal(float horizontal_value) {
        return Derived {horizontal_value, 0.f, horizontal_value, 0.f};
    }

    static Derived vertical(float vertical_value) {
        return Derived {0.f, vertical_value, 0.f, vertical_value};
    }

    Derived with_left(float left) const {
        return Derived {left, m_top, m_right, m_bottom};
    }

    Derived with_top(float top) const {
        return Derived {m_left, top, m_right, m_bottom};
    }

    Derived with_right(float right) const {
        return Derived {m_left, m_top, right, m_bottom};
    }

    Derived with_bottom(float bottom) const {
        return Derived {m_left, m_top, m_right, bottom};
    }

  protected:
    float m_left {0.f};
    float m_top {0.f};
    float m_right {0.f};
    float m_bottom {0.f};
};

class UiMargin: public UiBoxValue<UiMargin> {
  public:
    using UiBoxValue<UiMargin>::UiBoxValue;
    friend class UiNode;
};

class UiPadding: public UiBoxValue<UiPadding> {
  public:
    using UiBoxValue<UiPadding>::UiBoxValue;
    friend class UiNode;
};

class UiBorder: public UiBoxValue<UiBorder> {
  public:
    using UiBoxValue<UiBorder>::UiBoxValue;
    friend class UiNode;
    friend class Screen;
};

class UiPosition {
  public:
    static UiPosition left(float left_px) {
        UiPosition pos;
        pos.m_left = Val::Px(left_px);
        return pos;
    }

    static UiPosition top(float top_px) {
        UiPosition pos;
        pos.m_top = Val::Px(top_px);
        return pos;
    }

    static UiPosition right(float right_px) {
        UiPosition pos;
        pos.m_right = Val::Px(right_px);
        return pos;
    }

    static UiPosition bottom(float bottom_px) {
        UiPosition pos;
        pos.m_bottom = Val::Px(bottom_px);
        return pos;
    }

    static UiPosition left(const Val& left) {
        UiPosition pos;
        pos.m_left = left;
        return pos;
    }

    static UiPosition top(const Val& top) {
        UiPosition pos;
        pos.m_top = top;
        return pos;
    }

    static UiPosition right(const Val& right) {
        UiPosition pos;
        pos.m_right = right;
        return pos;
    }

    static UiPosition bottom(const Val& bottom) {
        UiPosition pos;
        pos.m_bottom = bottom;
        return pos;
    }

    UiPosition with_left(float left_px) {
        m_left = Val::Px(left_px);
        return *this;
    }

    UiPosition with_top(float top_px) {
        m_top = Val::Px(top_px);
        return *this;
    }

    UiPosition with_right(float right_px) {
        m_right = Val::Px(right_px);
        return *this;
    }

    UiPosition with_bottom(float bottom_px) {
        m_bottom = Val::Px(bottom_px);
        return *this;
    }

    UiPosition with_left(const Val& left) {
        m_left = left;
        return *this;
    }

    UiPosition with_top(const Val& top) {
        m_top = top;
        return *this;
    }

    UiPosition with_right(const Val& right) {
        m_right = right;
        return *this;
    }

    UiPosition with_bottom(const Val& bottom) {
        m_bottom = bottom;
        return *this;
    }

  private:
    friend class UiNode;

    Val m_left {Val::Px(0.f)};
    Val m_top {Val::Px(0.f)};
    Val m_right {Val::Px(0.f)};
    Val m_bottom {Val::Px(0.f)};
};

enum class AlignItems {
    Auto,
    FlexStart,
    Center,
    FlexEnd,
    Stretch,
    Baseline,
    SpaceBetween,
    SpaceAround
};

enum class JustifyContent {
    FlexStart,
    Center,
    FlexEnd,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
};

enum class AlignContent {
    Auto,
    FlexStart,
    Center,
    FlexEnd,
    Stretch,
    Baseline,
    SpaceBetween,
    SpaceAround
};

enum class FlexWrap { NoWrap, Wrap, WrapReverse };

enum PositionType { AutoLayout, Relative, Absolute };

enum FlexDirection { Row, Column, ColumnReverse };

struct UiInputState {
    Point mouse_position {0.0f, 0.0f};
    bool mouse_pressed = false;
    bool mouse_just_pressed = false;
    bool mouse_just_released = false;
};

class UiNode {
  public:
    UiNode() noexcept;
    virtual ~UiNode();
    UiNode(const UiNode&) = delete;
    UiNode& operator=(const UiNode&) = delete;
    UiNode(UiNode&& other) noexcept;
    UiNode& operator=(UiNode&& other) noexcept;

    template<typename T, typename... Args>
    T& add_node(Args&&... args) {
        auto node = std::make_unique<T>(std::forward<Args>(args)...);
        node->m_parent = this;
        auto* raw_node = node.get();

        add_child_internal(*raw_node);
        m_nodes.push_back(std::move(node));

        return *raw_node;
    }

    template<typename T>
    bool remove_node(T* node_to_remove) {
        auto it = std::find_if(
            m_nodes.begin(),
            m_nodes.end(),
            [node_to_remove](const auto& node) {
                return node.get() == node_to_remove;
            }
        );

        if (it != m_nodes.end()) {
            m_marked_for_removal.push_back({node_to_remove, it});
            return true;
        }

        return false;
    }

    // Visual properties
    UiNode& set_background_color(const Color& background_color) {
        m_background_color = background_color;
        return *this;
    }

    UiNode& set_corner_radius(float corner_radius) {
        m_corner_radius = corner_radius;
        return *this;
    }

    UiNode& set_border_color(const Color& border_color) {
        m_border_color = border_color;
        return *this;
    }

    UiNode& set_border_width(float border_width);
    UiNode& set_border(const UiBorder& border);

    UiNode& set_box_shadow(const BoxShadow& shadow) {
        m_box_shadow = shadow;
        return *this;
    }

    UiNode& set_visible(bool visible) {
        m_visible = visible;
        return *this;
    }

    bool is_visible() const {
        return m_visible;
    }

    UiNode* get_parent() const {
        return m_parent;
    }

    UiNode& set_width(float width_px);
    UiNode& set_width(const Val& val);
    UiNode& set_height(float height_px);
    UiNode& set_height(const Val& val);
    UiNode& set_flex_direction(FlexDirection direction);
    UiNode& set_padding(const UiPadding& padding);
    UiNode& set_margin(const UiMargin& margin);
    UiNode& set_flex_grow(float grow);
    UiNode& set_flex_wrap(FlexWrap wrap);
    UiNode& set_align_items(AlignItems align_items);
    UiNode& set_align_content(AlignContent align_content);
    UiNode& set_justify_content(JustifyContent justify_content);
    UiNode& set_position_type(PositionType position_type);
    UiNode& set_position(const Point& position);
    UiNode& set_position(const UiPosition& position);
    UiNode& set_gap(float gap);

    template<typename Func>
    void add_event_listener(Func&& listener) {
        using EventType = detail::first_arg_t<Func>;

        auto key = std::type_index(typeid(EventType));
        m_event_listeners[key].push_back(
            [f = std::forward<Func>(listener)](const void* e) {
                f(*static_cast<const EventType*>(e));
            }
        );
    }

    template<typename Func>
    void on(Func&& listener) {
        add_event_listener(std::forward<Func>(listener));
    }

    template<typename EventType>
    void trigger(const EventType& event) {
        auto key = std::type_index(typeid(EventType));
        auto it = m_event_listeners.find(key);
        if (it != m_event_listeners.end()) {
            for (auto& cb : it->second) {
                cb(&event);
            }
        }
    }

    template<typename EventType>
    void emit(const EventType& event) {
        trigger(event);
    }

  protected:
    UiNode* m_parent {nullptr};
    bool m_visible {true};

    Size computed_size() const;
    Point computed_position() const;

    virtual void update(const UiInputState& input);

    virtual void paint(Painter& painter) {
        painter.draw_rect(
            computed_position(),
            computed_size(),
            m_background_color,
            m_corner_radius,
            AntiAliasing::Enabled,
            m_border_color,
            BorderWidths {
                .left = m_border.m_left,
                .top = m_border.m_top,
                .right = m_border.m_right,
                .bottom = m_border.m_bottom
            },
            m_box_shadow
        );
    }

  private:
    friend class World;
    friend class UiRoot;
    friend class Screen;

    YGNode* m_yoga_node {nullptr};

    void calculate_layout(float width, float height);
    void add_child_internal(UiNode& child);
    void remove_child_internal(UiNode& child);

    bool m_hovered {false};
    bool m_pressed {false};

    std::
        flat_map<std::type_index, std::vector<std::function<void(const void*)>>>
            m_event_listeners;

    std::vector<
        std::pair<UiNode*, std::vector<std::unique_ptr<UiNode>>::iterator>>
        m_marked_for_removal;

    Color m_background_color {0.f, 0.f, 0.f, 0.f};
    float m_corner_radius {0.f};
    Color m_border_color {0.f, 0.f, 0.f, 0.f};
    UiBorder m_border {0.f, 0.f, 0.f, 0.f};
    BoxShadow m_box_shadow;

    std::vector<std::unique_ptr<UiNode>> m_nodes;
};

}  // namespace toolkit
