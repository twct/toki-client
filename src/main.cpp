#include <toolkit/app.h>
#include <toolkit/log.h>
#include <toolkit/screen.h>
#include <toolkit/ui/image.h>

#include <print>

using namespace toolkit;

class MainScreen: public Screen {
  public:
    void on_enter() override {
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
            event.target.set_background_color(Color::HOT_PINK);
        });

        header.on([](const UiMouseLeaveEvent& event) {
            event.target.set_background_color(Color::WHITE);
        });

        header.on([](const UiClickEvent& event) { log::info("Clicked Me!"); });

        auto& box = parent.add_node<UiNode>()
                        .set_width(200.f)
                        .set_height(100.f)
                        .set_margin(UiMargin::all(30.f))
                        .set_background_color(Color::WHITE)
                        .set_corner_radius(8.f)
                        .set_box_shadow(
                            BoxShadow()
                                .with_offset(4.f, 4.f)
                                .with_blur(12.f)
                                .with_color({0.f, 0.f, 0.f, 0.5f})
                        );

        auto& img = box.add_node<UiImage>(load<Image>("sakura.png"))
                        .set_fit(ImageFit::Cover)
                        .set_flex_grow(1.f);
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
