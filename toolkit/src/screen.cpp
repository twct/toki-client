#include <toolkit/screen.h>

#include <cassert>

namespace toolkit {

void Screen::update(const UiInputState& input) {
    update_node(m_root_node, input);
}

void Screen::update_node(UiNode& node, const UiInputState& input) {
    if (!node.is_visible()) return;

    node.update(input);

    for (auto& child : node.m_nodes) {
        update_node(*child, input);
    }
}

void Screen::paint_node(UiNode& node, Point parent_abs) {
    if (!node.is_visible()) return;

    m_painter.set_translation(parent_abs);
    node.paint(m_painter);

    Point this_abs = {
        parent_abs.x + node.computed_position().x,
        parent_abs.y + node.computed_position().y,
    };

    bool clip = node.m_corner_radius > 0.f && !node.m_nodes.empty();
    if (clip)
        m_painter.push_clip(node.computed_position(), node.computed_size(), node.m_corner_radius);

    for (auto& child : node.m_nodes) {
        paint_node(*child, this_abs);
    }

    if (clip)
        m_painter.pop_clip();
}

void Screen::render() {
    assert(m_renderer != nullptr);

    m_painter.clear();

    auto vp = m_renderer->viewport_size();
    m_root_node.set_width(vp.width);
    m_root_node.set_height(vp.height);
    m_root_node.calculate_layout(vp.width, vp.height);

    paint_node(m_root_node);

    m_renderer->submit(m_painter);
}

}  // namespace toolkit
