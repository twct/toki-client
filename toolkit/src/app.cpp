#include <SDL3/SDL.h>
#include <toolkit/app.h>

using namespace core;

namespace toolkit {

Result<int, Error> App::run(int argc, char** argv) {
    TRY_EXEC(m_window.open("Toki", 1600, 900));
    TRY_EXEC(m_renderer.init(m_window));

    SDL_Event event;

    while (m_running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    m_running = false;
                    break;
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    m_renderer.resize(event.window.data1, event.window.data2);
                    break;
                default:
                    break;
            }
        }

        m_renderer.render();
    }

    SDL_Quit();

    return 0;
}

}  // namespace toolkit
