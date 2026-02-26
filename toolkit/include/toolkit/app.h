#pragma once

#include <core/error.h>
#include <core/result.h>

#include "window.h"

namespace toolkit {

class App {
  public:
    core::Result<int, core::Error> run(int argc, char** argv);

  private:
    Window m_window;
    bool m_running {true};
};

}  // namespace toolkit