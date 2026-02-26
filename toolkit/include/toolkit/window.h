#pragma once

#include <SDL3/SDL_render.h>
#include <core/error.h>
#include <core/result.h>

namespace toolkit {

class Window {
  public:
    core::Result<core::Unit, core::Error>
    open(const std::string& title, unsigned int width, unsigned int height);

    const SDL_Window* sdl_window() const {
        return m_sdl_window;
    }

    const std::pair<int, int> size() const;

  private:
    SDL_Window* m_sdl_window {nullptr};
};

}  // namespace toolkit
