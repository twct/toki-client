#pragma once

#include <core/error.h>
#include <core/result.h>

#include <memory>
#include <stack>

#include "render/renderer.h"
#include "screen.h"
#include "window.h"

namespace toolkit {

class App {
  public:
    core::Result<int, core::Error> run(int argc, char** argv);

    template<typename T, typename... Args>
    App& set_default_screen(Args&&... args) {
        auto screen = std::make_unique<T>(std::forward<Args>(args)...);
        screen->__inject_context__(Screen::Context {.renderer = m_renderer});

        m_screens.push(std::move(screen));

        return *this;
    }

  private:
    Window m_window;
    Renderer m_renderer;
    std::stack<std::unique_ptr<Screen>> m_screens;
    bool m_running {true};
};

}  // namespace toolkit