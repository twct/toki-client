// main.cpp — Scaffold: SDL3 + OpenGL 3.3 + Dear ImGui + libmpv

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <mpv/client.h>
#include <mpv/render_gl.h>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <print>
#include <string>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

// ---------------------------------------------------------------------
// Globals (fine for a scaffold; you'll refactor these out later)
// ---------------------------------------------------------------------
static mpv_handle* g_mpv = nullptr;
static mpv_render_context* g_mpv_gl = nullptr;
static GLuint g_fbo = 0;
static GLuint g_fbo_tex = 0;
static int g_video_w = 1280;
static int g_video_h = 720;
static std::atomic<bool> g_wakeup {false};

// mpv calls this on its render thread when a new frame is ready
static void on_mpv_render_update(void* /*ctx*/) {
    g_wakeup = true;
}

// Process mpv async events (metadata changes, EOF, errors, etc.)
static void process_mpv_events() {
    while (true) {
        mpv_event* event = mpv_wait_event(g_mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;

        switch (event->event_id) {
            case MPV_EVENT_LOG_MESSAGE: {
                auto* msg = static_cast<mpv_event_log_message*>(event->data);
                std::printf("[mpv/%s] %s", msg->prefix, msg->text);
                break;
            }
            case MPV_EVENT_PROPERTY_CHANGE: {
                auto* prop = static_cast<mpv_event_property*>(event->data);
                if (std::strcmp(prop->name, "video-params/w") == 0
                    && prop->format == MPV_FORMAT_INT64) {
                    g_video_w =
                        static_cast<int>(*static_cast<int64_t*>(prop->data));
                } else if (std::strcmp(prop->name, "video-params/h") == 0
                           && prop->format == MPV_FORMAT_INT64) {
                    g_video_h =
                        static_cast<int>(*static_cast<int64_t*>(prop->data));
                }
                break;
            }
            case MPV_EVENT_FILE_LOADED:
                std::printf("[app] File loaded.\n");
                break;
            case MPV_EVENT_END_FILE:
                std::printf("[app] Playback ended.\n");
                break;
            default:
                break;
        }
    }
}

// (Re)create the FBO that mpv renders into
static void ensure_fbo(int w, int h) {
    if (w <= 0 || h <= 0)
        return;

    if (g_fbo) {
        glDeleteFramebuffers(1, &g_fbo);
        glDeleteTextures(1, &g_fbo_tex);
        g_fbo = 0;
        g_fbo_tex = 0;
    }

    glGenTextures(1, &g_fbo_tex);
    glBindTexture(GL_TEXTURE_2D, g_fbo_tex);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        w,
        h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenFramebuffers(1, &g_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        g_fbo_tex,
        0
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Load a file into mpv
static void load_file(const char* path) {
    const char* cmd[] = {"loadfile", path, nullptr};
    mpv_command_async(g_mpv, 0, cmd);
}

// ---------------------------------------------------------------------
// main
// ---------------------------------------------------------------------
int main(int argc, char* argv[]) {
    // --- SDL3 init ---------------------------------------------------
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Request OpenGL 3.3 Core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE
    );
#ifdef __APPLE__
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_FLAGS,
        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
    );
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

    SDL_Window* window = SDL_CreateWindow(
        "media-shelf",
        1600,
        900,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_ctx);
    SDL_GL_SetSwapInterval(1);  // vsync

    GLenum glew_err = glewInit();
    if (glew_err != GLEW_OK) {
        std::fprintf(
            stderr,
            "glewInit failed: %s\n",
            glewGetErrorString(glew_err)
        );
        return 1;
    }

    // --- Dear ImGui init ---------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForOpenGL(window, gl_ctx);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // --- mpv init ----------------------------------------------------
    g_mpv = mpv_create();
    if (!g_mpv) {
        std::fprintf(stderr, "mpv_create() failed\n");
        return 1;
    }

    mpv_set_option_string(g_mpv, "vo", "libmpv");
    mpv_set_option_string(g_mpv, "hwdec", "auto");
    mpv_set_option_string(g_mpv, "keep-open", "yes");
    mpv_set_option_string(g_mpv, "idle", "yes");
    mpv_request_log_messages(g_mpv, "warn");

    if (mpv_initialize(g_mpv) < 0) {
        std::fprintf(stderr, "mpv_initialize() failed\n");
        return 1;
    }

    // Observe video dimensions so we can resize the FBO dynamically
    mpv_observe_property(g_mpv, 0, "video-params/w", MPV_FORMAT_INT64);
    mpv_observe_property(g_mpv, 0, "video-params/h", MPV_FORMAT_INT64);

    // --- mpv OpenGL render context -----------------------------------
    mpv_opengl_init_params gl_init_params {
        .get_proc_address = [](void* /*ctx*/, const char* name) -> void* {
            return reinterpret_cast<void*>(SDL_GL_GetProcAddress(name));
        },
        .get_proc_address_ctx = nullptr,
    };

    // int adv_control = 1;
    int adv_control = 0;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE,
         const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &adv_control},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    if (mpv_render_context_create(&g_mpv_gl, g_mpv, params) < 0) {
        std::fprintf(stderr, "mpv_render_context_create() failed\n");
        return 1;
    }

    mpv_render_context_set_update_callback(
        g_mpv_gl,
        on_mpv_render_update,
        nullptr
    );

    ensure_fbo(g_video_w, g_video_h);

    if (argc > 1) {
        load_file(argv[1]);
    }

    int last_fbo_w = g_video_w;
    int last_fbo_h = g_video_h;

    // --- Main loop ---------------------------------------------------
    bool running = true;
    while (running) {
        // -- Events ---------------------------------------------------
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL3_ProcessEvent(&ev);

            if (ev.type == SDL_EVENT_QUIT)
                running = false;

            if (ev.type == SDL_EVENT_DROP_FILE) {
                load_file(ev.drop.data);
            }

            if (!io.WantCaptureKeyboard && ev.type == SDL_EVENT_KEY_DOWN) {
                switch (ev.key.key) {
                    case SDLK_SPACE: {
                        const char* cmd[] = {"cycle", "pause", nullptr};
                        mpv_command_async(g_mpv, 0, cmd);
                        break;
                    }
                    case SDLK_LEFT: {
                        const char* cmd[] = {"seek", "-5", nullptr};
                        mpv_command_async(g_mpv, 0, cmd);
                        break;
                    }
                    case SDLK_RIGHT: {
                        const char* cmd[] = {"seek", "5", nullptr};
                        mpv_command_async(g_mpv, 0, cmd);
                        break;
                    }
                    case SDLK_Q:
                        running = false;
                        break;
                    default:
                        break;
                }
            }
        }

        process_mpv_events();

        // Resize FBO if video dimensions changed
        if (g_video_w != last_fbo_w || g_video_h != last_fbo_h) {
            ensure_fbo(g_video_w, g_video_h);
            last_fbo_w = g_video_w;
            last_fbo_h = g_video_h;
        }

        // -- Render mpv frame into FBO --------------------------------
        uint64_t flags = mpv_render_context_update(g_mpv_gl);
        if (flags & MPV_RENDER_UPDATE_FRAME) {
            mpv_opengl_fbo fbo_params {
                .fbo = static_cast<int>(g_fbo),
                .w = g_video_w,
                .h = g_video_h,
            };
            int flip_y = 1;

            mpv_render_param render_params[] = {
                {MPV_RENDER_PARAM_OPENGL_FBO, &fbo_params},
                {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
                {MPV_RENDER_PARAM_INVALID, nullptr},
            };

            mpv_render_context_render(g_mpv_gl, render_params);
        }

        // -- ImGui frame ----------------------------------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // --- Player window -------------------------------------------
        ImGui::SetNextWindowSize(ImVec2(900, 560), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Player")) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float player_h = avail.y - 60.0f;
            if (player_h < 100.0f)
                player_h = 100.0f;

            float aspect =
                static_cast<float>(g_video_w) / static_cast<float>(g_video_h);
            float display_w = avail.x;
            float display_h = display_w / aspect;
            if (display_h > player_h) {
                display_h = player_h;
                display_w = display_h * aspect;
            }

            float offset_x = (avail.x - display_w) * 0.5f;
            if (offset_x > 0.0f)
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);

            ImGui::Image(
                static_cast<ImTextureID>(static_cast<intptr_t>(g_fbo_tex)),
                ImVec2(display_w, display_h),
                ImVec2(0, 0),
                ImVec2(1, 1)
            );

            // --- Transport controls ----------------------------------
            ImGui::Separator();

            double time_pos = 0.0, duration = 0.0;
            int paused = 0;
            mpv_get_property(g_mpv, "time-pos", MPV_FORMAT_DOUBLE, &time_pos);
            mpv_get_property(g_mpv, "duration", MPV_FORMAT_DOUBLE, &duration);
            mpv_get_property(g_mpv, "pause", MPV_FORMAT_FLAG, &paused);

            if (ImGui::Button(paused ? "  Play  " : " Pause  ")) {
                const char* cmd[] = {"cycle", "pause", nullptr};
                mpv_command_async(g_mpv, 0, cmd);
            }
            ImGui::SameLine();

            ImGui::SetNextItemWidth(-120.0f);
            float pos_f = (duration > 0.0)
                ? static_cast<float>(time_pos / duration)
                : 0.0f;
            if (ImGui::SliderFloat("##timeline", &pos_f, 0.0f, 1.0f, "")) {
                double seek_to = static_cast<double>(pos_f) * duration;
                mpv_set_property_async(
                    g_mpv,
                    0,
                    "time-pos",
                    MPV_FORMAT_DOUBLE,
                    &seek_to
                );
            }
            ImGui::SameLine();

            auto fmt_time = [](double t) -> std::string {
                int h = static_cast<int>(t) / 3600;
                int m = (static_cast<int>(t) % 3600) / 60;
                int s = static_cast<int>(t) % 60;
                char buf[32];
                if (h > 0)
                    std::snprintf(buf, sizeof(buf), "%d:%02d:%02d", h, m, s);
                else
                    std::snprintf(buf, sizeof(buf), "%02d:%02d", m, s);
                return buf;
            };
            ImGui::Text(
                "%s / %s",
                fmt_time(time_pos).c_str(),
                fmt_time(duration).c_str()
            );
        }
        ImGui::End();

        // --- Library / sidebar (stub) --------------------------------
        ImGui::SetNextWindowSize(ImVec2(350, 560), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Library")) {
            ImGui::Text("Shows & Seasons");
            ImGui::Separator();
            ImGui::TextWrapped(
                "TODO: series list, season grouping, episode list.\n"
                "Drag-and-drop files onto the window to play."
            );

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::Text("Episode Notes");
            ImGui::Separator();
            static char notes_buf[4096] = "";
            ImGui::InputTextMultiline(
                "##notes",
                notes_buf,
                sizeof(notes_buf),
                ImVec2(-1, 200)
            );

            if (ImGui::Button("Add Timestamp Note")) {
                double t = 0.0;
                mpv_get_property(g_mpv, "time-pos", MPV_FORMAT_DOUBLE, &t);
                int len = static_cast<int>(std::strlen(notes_buf));
                std::snprintf(
                    notes_buf + len,
                    sizeof(notes_buf) - len,
                    "\n[%02d:%02d] ",
                    static_cast<int>(t) / 60,
                    static_cast<int>(t) % 60
                );
            }
        }
        ImGui::End();

        // --- Render --------------------------------------------------
        ImGui::Render();
        int fb_w, fb_h;
        SDL_GetWindowSizeInPixels(window, &fb_w, &fb_h);
        glViewport(0, 0, fb_w, fb_h);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // --- Cleanup -----------------------------------------------------
    mpv_render_context_free(g_mpv_gl);
    mpv_terminate_destroy(g_mpv);

    if (g_fbo) {
        glDeleteFramebuffers(1, &g_fbo);
        glDeleteTextures(1, &g_fbo_tex);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
