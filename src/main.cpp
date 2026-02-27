#include <toolkit/app.h>
#include <toolkit/log.h>
#include <toolkit/screen.h>

#include <print>

using namespace toolkit;

class MainScreen: public Screen {
  public:
    MainScreen() {
        log::info("Hello from MainScreen!");

        auto& parent = add_node<UiNode>()
                           .set_background_color(Color::RED)
                           .set_flex_grow(1.f);

        auto& header = parent.add_node<UiNode>()
                           .set_padding(UiPadding::all(15.f))
                           .set_margin(UiMargin::all(15.f))
                           .set_background_color(Color::WHITE)
                           .set_border(UiBorder::vertical(12.f))
                           .set_border_color(Color::BLACK);

        header.on([](const UiMouseEnterEvent& event) {
            event.target.set_background_color(Color::PINK);
        });

        header.on([](const UiMouseLeaveEvent& event) {
            event.target.set_background_color(Color::WHITE);
        });

        header.on([](const UiClickEvent& event) { log::info("Clicked Me!"); });
    }
};

int main(int argc, char** argv) {
    return App()
        .set_default_screen<MainScreen>()
        .run(argc, argv)
        .unwrap_or_else([](const auto& e) {
            std::println(
                stderr,
                "Application ran into a fatal problem: {}",
                e.message()
            );
            return 1;
        });
}
