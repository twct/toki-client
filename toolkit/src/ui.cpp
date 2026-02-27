#include <toolkit/ui.h>
#include <yoga/YGNodeStyle.h>
#include <yoga/Yoga.h>

#include <cmath>

namespace toolkit {

template<typename T>
YGAlign align_to_yoga(T align_type) {
    switch (align_type) {
        case T::Auto:
            return YGAlignAuto;
        case T::FlexStart:
            return YGAlignFlexStart;
        case T::Center:
            return YGAlignCenter;
        case T::FlexEnd:
            return YGAlignFlexEnd;
        case T::Stretch:
            return YGAlignStretch;
        case T::Baseline:
            return YGAlignBaseline;
        case T::SpaceBetween:
            return YGAlignSpaceBetween;
        case T::SpaceAround:
            return YGAlignSpaceAround;
    }
}

YGJustify justify_content_to_yoga(JustifyContent justify_content) {
    switch (justify_content) {
        case JustifyContent::FlexStart:
            return YGJustifyFlexStart;
        case JustifyContent::Center:
            return YGJustifyCenter;
        case JustifyContent::FlexEnd:
            return YGJustifyFlexEnd;
        case JustifyContent::SpaceBetween:
            return YGJustifySpaceBetween;
        case JustifyContent::SpaceAround:
            return YGJustifySpaceAround;
        case JustifyContent::SpaceEvenly:
            return YGJustifySpaceEvenly;
    }
}

UiNode::UiNode() noexcept {
    m_yoga_node = YGNodeNew();
}

UiNode::~UiNode() {
    if (m_yoga_node) {
        YGNodeFree(m_yoga_node);
    }
}

UiNode::UiNode(UiNode&& other) noexcept :
    m_parent(other.m_parent),
    m_visible(other.m_visible),
    m_yoga_node(other.m_yoga_node),
    m_marked_for_removal(std::move(other.m_marked_for_removal)),
    m_background_color(other.m_background_color),
    m_corner_radius(other.m_corner_radius),
    m_nodes(std::move(other.m_nodes)) {
    other.m_yoga_node = nullptr;
    other.m_parent = nullptr;
}

UiNode& UiNode::operator=(UiNode&& other) noexcept {
    if (this != &other) {
        if (m_yoga_node) {
            YGNodeFree(m_yoga_node);
        }
        m_parent = other.m_parent;
        m_visible = other.m_visible;
        m_yoga_node = other.m_yoga_node;
        m_marked_for_removal = std::move(other.m_marked_for_removal);
        m_background_color = other.m_background_color;
        m_corner_radius = other.m_corner_radius;
        m_nodes = std::move(other.m_nodes);

        other.m_yoga_node = nullptr;
        other.m_parent = nullptr;
    }
    return *this;
}

UiNode& UiNode::set_width(const Val& val) {
    switch (val.m_type) {
        case ValType::Px:
            YGNodeStyleSetWidth(m_yoga_node, val.m_amount);
            break;
        case ValType::Percent:
            YGNodeStyleSetWidthPercent(m_yoga_node, val.m_amount);
            break;
    }
    return *this;
}

UiNode& UiNode::set_width(float width_px) {
    return set_width(Val::Px(width_px));
}

UiNode& UiNode::set_height(const Val& val) {
    switch (val.m_type) {
        case ValType::Px:
            YGNodeStyleSetHeight(m_yoga_node, val.m_amount);
            break;
        case ValType::Percent:
            YGNodeStyleSetHeightPercent(m_yoga_node, val.m_amount);
            break;
    }
    return *this;
}

UiNode& UiNode::set_height(float height_px) {
    return set_height(Val::Px(height_px));
}

UiNode& UiNode::set_flex_direction(FlexDirection direction) {
    switch (direction) {
        case FlexDirection::Row:
            YGNodeStyleSetFlexDirection(m_yoga_node, YGFlexDirectionRow);
            break;
        case FlexDirection::Column:
            YGNodeStyleSetFlexDirection(m_yoga_node, YGFlexDirectionColumn);
            break;
        case FlexDirection::ColumnReverse:
            YGNodeStyleSetFlexDirection(
                m_yoga_node,
                YGFlexDirectionColumnReverse
            );
            break;
    }
    return *this;
}

UiNode& UiNode::set_padding(const UiPadding& padding) {
    YGNodeStyleSetPadding(m_yoga_node, YGEdgeLeft, padding.m_left);
    YGNodeStyleSetPadding(m_yoga_node, YGEdgeTop, padding.m_top);
    YGNodeStyleSetPadding(m_yoga_node, YGEdgeRight, padding.m_right);
    YGNodeStyleSetPadding(m_yoga_node, YGEdgeBottom, padding.m_bottom);
    return *this;
}

UiNode& UiNode::set_margin(const UiMargin& margin) {
    YGNodeStyleSetMargin(m_yoga_node, YGEdgeLeft, margin.m_left);
    YGNodeStyleSetMargin(m_yoga_node, YGEdgeTop, margin.m_top);
    YGNodeStyleSetMargin(m_yoga_node, YGEdgeRight, margin.m_right);
    YGNodeStyleSetMargin(m_yoga_node, YGEdgeBottom, margin.m_bottom);
    return *this;
}

UiNode& UiNode::set_flex_grow(float grow) {
    YGNodeStyleSetFlexGrow(m_yoga_node, grow);
    return *this;
}

UiNode& UiNode::set_flex_wrap(FlexWrap wrap) {
    switch (wrap) {
        case FlexWrap::NoWrap:
            YGNodeStyleSetFlexWrap(m_yoga_node, YGWrapNoWrap);
            break;
        case FlexWrap::Wrap:
            YGNodeStyleSetFlexWrap(m_yoga_node, YGWrapWrap);
            break;
        case FlexWrap::WrapReverse:
            YGNodeStyleSetFlexWrap(m_yoga_node, YGWrapWrapReverse);
            break;
    }
    return *this;
}

UiNode& UiNode::set_align_items(AlignItems align_items) {
    YGNodeStyleSetAlignItems(m_yoga_node, align_to_yoga(align_items));
    return *this;
}

UiNode& UiNode::set_align_content(AlignContent align_content) {
    YGNodeStyleSetAlignContent(m_yoga_node, align_to_yoga(align_content));
    return *this;
}

UiNode& UiNode::set_justify_content(JustifyContent justify_content) {
    YGNodeStyleSetJustifyContent(
        m_yoga_node,
        justify_content_to_yoga(justify_content)
    );
    return *this;
}

UiNode& UiNode::set_position_type(PositionType position_type) {
    auto get_yg_position_type = [](PositionType type) {
        switch (type) {
            case PositionType::AutoLayout:
                return YGPositionTypeStatic;
            case PositionType::Absolute:
                return YGPositionTypeAbsolute;
            case PositionType::Relative:
                return YGPositionTypeRelative;
        }
        return YGPositionTypeStatic;
    };
    YGNodeStyleSetPositionType(
        m_yoga_node,
        get_yg_position_type(position_type)
    );
    return *this;
}

UiNode& UiNode::set_position(const Point& position) {
    YGNodeStyleSetPosition(m_yoga_node, YGEdgeLeft, position.x);
    YGNodeStyleSetPosition(m_yoga_node, YGEdgeTop, position.y);
    return *this;
}

UiNode& UiNode::set_position(const UiPosition& position) {
    auto set_edge_position = [this](YGEdge edge, const Val& val) {
        switch (val.m_type) {
            case ValType::Px:
                YGNodeStyleSetPositionType(m_yoga_node, YGPositionTypeAbsolute);
                YGNodeStyleSetPosition(m_yoga_node, edge, val.m_amount);
                break;
            case ValType::Percent:
                YGNodeStyleSetPositionType(m_yoga_node, YGPositionTypeAbsolute);
                YGNodeStyleSetPositionPercent(m_yoga_node, edge, val.m_amount);
                break;
        }
    };

    set_edge_position(YGEdgeLeft, position.m_left);
    set_edge_position(YGEdgeTop, position.m_top);
    set_edge_position(YGEdgeRight, position.m_right);
    set_edge_position(YGEdgeBottom, position.m_bottom);

    return *this;
}

UiNode& UiNode::set_border_width(float border_width) {
    m_border = UiBorder::all(border_width);
    YGNodeStyleSetBorder(m_yoga_node, YGEdgeAll, border_width);
    return *this;
}

UiNode& UiNode::set_border(const UiBorder& border) {
    m_border = border;
    YGNodeStyleSetBorder(m_yoga_node, YGEdgeLeft, border.m_left);
    YGNodeStyleSetBorder(m_yoga_node, YGEdgeTop, border.m_top);
    YGNodeStyleSetBorder(m_yoga_node, YGEdgeRight, border.m_right);
    YGNodeStyleSetBorder(m_yoga_node, YGEdgeBottom, border.m_bottom);
    return *this;
}

UiNode& UiNode::set_gap(float gap) {
    YGNodeStyleSetGap(m_yoga_node, YGGutterAll, gap);
    return *this;
}

void UiNode::calculate_layout(float width, float height) {
    YGNodeCalculateLayout(m_yoga_node, width, height, YGDirectionLTR);
}

Point UiNode::computed_position() const {
    return {YGNodeLayoutGetLeft(m_yoga_node), YGNodeLayoutGetTop(m_yoga_node)};
}

Size UiNode::computed_size() const {
    return {
        YGNodeLayoutGetWidth(m_yoga_node),
        YGNodeLayoutGetHeight(m_yoga_node)
    };
}

void UiNode::add_child_internal(UiNode& child) {
    YGNodeInsertChild(
        m_yoga_node,
        child.m_yoga_node,
        YGNodeGetChildCount(m_yoga_node)
    );
}

void UiNode::remove_child_internal(UiNode& child) {
    YGNodeRemoveChild(m_yoga_node, child.m_yoga_node);
}

bool is_point_inside(
    const Point& point,
    const Point& position,
    const Size& size
) {
    return (point.x >= position.x && point.x <= position.x + size.width)
        && (point.y >= position.y && point.y <= position.y + size.height);
}

}