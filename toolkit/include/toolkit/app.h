#pragma once

#include <core/error.h>
#include <core/result.h>

#include <memory>
#include <stack>

#include "assetserver.h"
#include "render/renderer.h"
#include "screen.h"
#include "window.h"

namespace toolkit {

class App {
  public:
    core::Result<int, core::Error> run(int argc, char** argv);

    template<typename T, typename... Args>
    App& set_default_screen(Args&&... args) {
        m_asset_server.init(m_renderer);
        auto screen = std::make_unique<T>(std::forward<Args>(args)...);
        screen->__inject_context__(Screen::Context {
            .renderer = m_renderer,
            .asset_server = m_asset_server,
        });

        m_screens.push(std::move(screen));

        return *this;
    }

    template<typename T>
    Handle<T> load(const std::string& path) {
        return m_asset_server.load<T>(path);
    }

  private:
    Window m_window;
    Renderer m_renderer;
    AssetServer m_asset_server;
    std::stack<std::unique_ptr<Screen>> m_screens;
    bool m_running {true};
};

}  // namespace toolkit