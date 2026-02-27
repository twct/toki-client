#pragma once

#include "render/renderer.h"
#include "ui.h"
#include "ui/painter.h"

namespace toolkit {

class Screen {
  public:
    virtual ~Screen() = default;

    template<typename T, typename... Args>
    T& add_node(Args&&... args) {
        return m_root_node.add_node<T>(std::forward<Args>(args)...);
    }

  private:
    friend class App;

    UiNode m_root_node;
    Painter m_painter;

    struct Context {
        Renderer& renderer;
    };

    void __inject_context__(const Context& context) {
        m_renderer = &context.renderer;
    }

    Renderer* m_renderer {nullptr};

    void render();
    void paint_node(UiNode& node);
};

}  // namespace toolkit