#pragma once

#include <core/error.h>
#include <core/result.h>

#include "render/renderer.h"
#include "window.h"

namespace toolkit {

class App {
  public:
    core::Result<int, core::Error> run(int argc, char** argv);

  private:
    Window m_window;
    Renderer m_renderer;
    bool m_running {true};
};

}  // namespace toolkit