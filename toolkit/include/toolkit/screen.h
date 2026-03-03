#pragma once

#include "assetserver.h"
#include "render/renderer.h"
#include "ui.h"
#include "ui/painter.h"

namespace toolkit {

class Screen {
  public:
    virtual ~Screen() = default;

    virtual void on_enter() {}

    template<typename T, typename... Args>
    T& add_node(Args&&... args) {
        return m_root_node.add_node<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    Handle<T> load(const std::string& path) {
        return m_asset_server->load<T>(path);
    }

  private:
    friend class App;

    UiNode m_root_node;
    Painter m_painter;

    struct Context {
        Renderer& renderer;
        AssetServer& asset_server;
    };

    void __inject_context__(const Context& context) {
        m_renderer = &context.renderer;
        m_asset_server = &context.asset_server;
        on_enter();
    }

    Renderer* m_renderer {nullptr};
    AssetServer* m_asset_server {nullptr};

    void update(const UiInputState& input);
    void update_node(UiNode& node, const UiInputState& input);
    void render();
    void paint_node(UiNode& node, Point parent_abs = {0.f, 0.f});
};

}  // namespace toolkit