#pragma once

#include <functional>
#include <stack>
#include <tuple>
#include <vector>

#include "assetserver.h"
#include "render/renderer.h"
#include "ui.h"
#include "ui/painter.h"

namespace toolkit {

class ScreenManager;

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

    template<typename T, typename... Args>
    void push_screen(Args&&... args);

    inline void pop_screen();

  private:
    friend class App;
    friend class ScreenManager;

    UiNode m_root_node;
    Painter m_painter;

    struct Context {
        Renderer& renderer;
        AssetServer& asset_server;
        ScreenManager& screen_manager;
    };

    void __inject_context__(const Context& context) {
        m_renderer = &context.renderer;
        m_asset_server = &context.asset_server;
        m_screen_manager = &context.screen_manager;
        on_enter();
    }

    Renderer* m_renderer {nullptr};
    AssetServer* m_asset_server {nullptr};
    ScreenManager* m_screen_manager {nullptr};

    void update(const UiInputState& input);
    void update_node(
        UiNode& node,
        const UiInputState& input,
        Point parent_abs = {0.f, 0.f}
    );
    void render();
    void paint_node(UiNode& node, Point parent_abs = {0.f, 0.f});
};

class ScreenManager {
  public:
    template<typename T, typename... Args>
    void push(Args&&... args) {
        m_pending.emplace_back(
            [this, tup = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                auto screen = std::apply(
                    [](auto&&... a) {
                        return std::make_unique<T>(std::forward<decltype(a)>(a)...);
                    },
                    std::move(tup)
                );
                screen->__inject_context__(Screen::Context {
                    .renderer = m_renderer,
                    .asset_server = m_asset_server,
                    .screen_manager = *this,
                });
                m_screens.push(std::move(screen));
            }
        );
    }

    void pop() {
        m_pending.emplace_back([this]() {
            if (!m_screens.empty()) {
                m_screens.pop();
            }
        });
    }

    void flush() {
        while (!m_pending.empty()) {
            auto pending = std::move(m_pending);
            for (auto& cmd : pending) {
                cmd();
            }
        }
    }

    Screen* current() {
        return m_screens.empty() ? nullptr : m_screens.top().get();
    }

    bool empty() const {
        return m_screens.empty();
    }

  private:
    friend class App;

    explicit ScreenManager(Renderer& renderer, AssetServer& asset_server) :
        m_renderer(renderer),
        m_asset_server(asset_server) {}

    Renderer& m_renderer;
    AssetServer& m_asset_server;
    std::stack<std::unique_ptr<Screen>> m_screens;
    std::vector<std::function<void()>> m_pending;
};

template<typename T, typename... Args>
void Screen::push_screen(Args&&... args) {
    m_screen_manager->push<T>(std::forward<Args>(args)...);
}

inline void Screen::pop_screen() {
    m_screen_manager->pop();
}

}  // namespace toolkit