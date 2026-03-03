#include <SDL3/SDL.h>
#include <toolkit/app.h>

using namespace core;

namespace toolkit {

Result<int, Error> App::run(int argc, char** argv) {
    TRY_EXEC(m_window.open("Toki", 1600, 900));
    TRY_EXEC(m_renderer.init(m_window));

    SDL_Event event;
    UiInputState input_state;

    // TODO: replace with SDL user events pushed from worker threads so the loop
    // can sleep when idle and wake on async completions (e.g. image uploads).
    // For now we redraw every frame; FIFO present mode keeps this vsync-locked.
    //
    // bool needs_redraw = true;
    // auto handle_event = [&](const SDL_Event& e) {
    //     switch (e.type) {
    //         case SDL_EVENT_QUIT:          m_running = false; break;
    //         case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: ... needs_redraw = true; break;
    //         ...
    //         case SDL_USER_EVENT_REDRAW:   needs_redraw = true; break;
    //     }
    // };

    auto handle_event = [&](const SDL_Event& e) {
        switch (e.type) {
            case SDL_EVENT_QUIT:
                m_running = false;
                break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                m_renderer.resize(e.window.data1, e.window.data2);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                input_state.mouse_position = {e.motion.x, e.motion.y};
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    input_state.mouse_pressed = true;
                    input_state.mouse_just_pressed = true;
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    input_state.mouse_pressed = false;
                    input_state.mouse_just_released = true;
                }
                break;
            default:
                break;
        }
    };

    while (m_running) {
        while (SDL_PollEvent(&event)) {
            handle_event(event);
        }

        if (m_running) {
            if (!m_screens.empty()) {
                m_screens.top()->update(input_state);
                m_screens.top()->render();
            }
            m_renderer.render();

            input_state.mouse_just_pressed = false;
            input_state.mouse_just_released = false;
        }
    }

    SDL_Quit();

    return 0;
}

}  // namespace toolkit
