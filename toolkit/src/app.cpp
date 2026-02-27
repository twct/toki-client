#include <SDL3/SDL.h>
#include <toolkit/app.h>

using namespace core;

namespace toolkit {

Result<int, Error> App::run(int argc, char** argv) {
    TRY_EXEC(m_window.open("Toki", 1600, 900));
    TRY_EXEC(m_renderer.init(m_window));

    SDL_Event event;
    bool needs_redraw = true;

    auto handle_event = [&](const SDL_Event& e) {
        switch (e.type) {
            case SDL_EVENT_QUIT:
                m_running = false;
                break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                m_renderer.resize(e.window.data1, e.window.data2);
                needs_redraw = true;
                break;
            case SDL_EVENT_WINDOW_EXPOSED:
            case SDL_EVENT_WINDOW_RESTORED:
                needs_redraw = true;
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
            case SDL_EVENT_MOUSE_MOTION:
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            case SDL_EVENT_MOUSE_WHEEL:
                needs_redraw = true;
                break;
            default:
                break;
        }
    };

    while (m_running) {
        // Sleep when idle; wake up only when an event arrives.
        if (!needs_redraw) {
            if (!SDL_WaitEvent(&event)) {
                continue;
            }
            handle_event(event);
        }

        while (SDL_PollEvent(&event)) {
            handle_event(event);
        }

        if (m_running && needs_redraw) {
            m_renderer.render();
            needs_redraw = false;
        }
    }

    SDL_Quit();

    return 0;
}

}  // namespace toolkit
