// clang-format off
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
// clang-format on

#include <SDL3/SDL.h>
#include <toolkit/window.h>

using namespace core;

namespace toolkit {

Result<Unit, Error> Window::open(
    const std::string& title,
    unsigned int width,
    unsigned int height
) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return Error("SDL_Init failed: {}", SDL_GetError());
    }

    m_sdl_window = SDL_CreateWindow(
        title.c_str(),
        width,
        height,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if (!m_sdl_window) {
        return Error("SDL_CreateWindow failed: {}", SDL_GetError());
    }

    return Unit {};
}

const std::pair<int, int> Window::size() const {
    int w, h;
    SDL_GetWindowSize(const_cast<SDL_Window*>(m_sdl_window), &w, &h);

    return {w, h};
}

}  // namespace toolkit
