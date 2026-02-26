#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <core/SkCanvas.h>
#include <core/SkColor.h>
#include <core/SkColorSpace.h>
#include <core/SkFont.h>
#include <core/SkFontMgr.h>
#include <core/SkRRect.h>
#include <core/SkSurface.h>
#include <core/SkTypeface.h>
#include <gpu/ganesh/GrBackendSurface.h>
#include <gpu/ganesh/GrDirectContext.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <gpu/ganesh/gl/GrGLBackendSurface.h>
#include <gpu/ganesh/gl/GrGLDirectContext.h>
#include <gpu/ganesh/gl/GrGLInterface.h>
#include <gpu/ganesh/gl/GrGLTypes.h>
#include <gpu/ganesh/gl/glx/GrGLMakeGLXInterface.h>
#include <ports/SkFontMgr_directory.h>

static constexpr int kWidth = 800;
static constexpr int kHeight = 600;

int main(int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE
    );
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(
        "toki",
        kWidth,
        kHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return 1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_Log("glewInit failed");
        return 1;
    }

    // Skia GL context
    auto interface = GrGLInterfaces::MakeGLX();
    auto gr_context = GrDirectContexts::MakeGL(interface);
    if (!gr_context) {
        SDL_Log("Failed to create Skia GrDirectContext");
        return 1;
    }

    // Load a system font directly from file
    auto font_mgr = SkFontMgr_New_Custom_Directory("/usr/share/fonts/TTF/");
    sk_sp<SkTypeface> typeface =
        font_mgr->makeFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf");
    if (!typeface) {
        SDL_Log("Failed to load typeface from file");
        return 1;
    }
    SkFont font(typeface, 36);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            if (event.type == SDL_EVENT_KEY_DOWN
                && event.key.key == SDLK_ESCAPE)
                running = false;
        }

        // Set up Skia render target from the current GL framebuffer
        GrGLFramebufferInfo fb_info;
        fb_info.fFBOID = 0;
        fb_info.fFormat = GL_RGBA8;

        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);

        auto backend_rt = GrBackendRenderTargets::MakeGL(w, h, 0, 8, fb_info);
        auto surface = SkSurfaces::WrapBackendRenderTarget(
            gr_context.get(),
            backend_rt,
            kBottomLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType,
            SkColorSpace::MakeSRGB(),
            nullptr
        );

        if (!surface) {
            SDL_Log("Failed to create Skia surface");
            break;
        }

        SkCanvas* canvas = surface->getCanvas();

        // -- Draw --
        canvas->clear(SK_ColorDKGRAY);

        // A rounded rectangle
        SkPaint rect_paint;
        rect_paint.setColor(SkColorSetRGB(0x42, 0xA5, 0xF5));
        rect_paint.setAntiAlias(true);

        SkRRect rrect;
        rrect.setRectXY(SkRect::MakeXYWH(100, 100, 300, 200), 20, 20);
        canvas->drawRRect(rrect, rect_paint);

        // A circle
        SkPaint circle_paint;
        circle_paint.setColor(SkColorSetRGB(0xEF, 0x53, 0x50));
        circle_paint.setAntiAlias(true);
        canvas->drawCircle(550, 300, 80, circle_paint);

        // Some text
        SkPaint text_paint;
        text_paint.setColor(SK_ColorWHITE);
        text_paint.setAntiAlias(true);
        canvas->drawString("Hello, Skia!", 250, 450, font, text_paint);

        // Flush and present
        gr_context->flushAndSubmit(surface.get());
        SDL_GL_SwapWindow(window);
    }

    gr_context->abandonContext();
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
