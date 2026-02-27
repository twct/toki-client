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

void Screen::paint_node(UiNode& node) {
    if (!node.is_visible()) return;

    node.paint(m_painter);

    for (auto& child : node.m_nodes) {
        paint_node(*child);
    }
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
