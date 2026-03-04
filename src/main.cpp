#include <toolkit/app.h>
#include <toolkit/log.h>
#include <toolkit/screen.h>
#include <toolkit/ui/image.h>

#include <print>

using namespace toolkit;

class ImageCard: public UiNode {
  public:
    template<typename Func>
    ImageCard(const Handle<Image>& image, Func&& func) {
        auto& box = set_width(200.f)
                        .set_height(100.f)
                        .set_background_color(Color::WHITE)
                        .set_corner_radius(8.f)
                        .set_box_shadow(
                            BoxShadow()
                                .with_offset(4.f, 4.f)
                                .with_blur(12.f)
                                .with_color({0.f, 0.f, 0.f, 0.5f})
                        );

        box.add_node<UiImage>(image)
            .set_fit(ImageFit::Cover)
            .set_flex_grow(1.f);

        box.on([func = std::move(func)](const UiClickEvent& event) { func(); });
    }
};

class PlayerScreen: public Screen {
  public:
    PlayerScreen(int index) : m_index(index) {
        log::info("Hello from PlayerScreen! {}", index);

        auto& container = add_node<UiNode>()
                              .set_flex_grow(1.f)
                              .set_align_items(AlignItems::Center)
                              .set_justify_content(JustifyContent::Center);

        auto& box = container.add_node<UiNode>()
                        .set_width(400.f)
                        .set_height(400.f)
                        .set_background_color(s_default_color);

        box.on([](const UiMouseEnterEvent& event) {
            event.target.set_background_color(Color::WHITE);
        });

        box.on([](const UiMouseLeaveEvent& event) {
            event.target.set_background_color(s_default_color);
        });

        box.on([this](const UiClickEvent& event) { pop_screen(); });
    }

  private:
    int m_index {0};
    inline static Color s_default_color {Color::HOT_PINK};
};

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

        auto& cards = parent.add_node<UiNode>()
                          .set_flex_grow(1.f)
                          .set_gap(20.f)
                          .set_margin(UiMargin::all(25.f))
                          .set_flex_direction(FlexDirection::Row)
                          .set_flex_wrap(FlexWrap::Wrap);

        for (int i = 0; i < 12; ++i) {
            cards.add_node<ImageCard>(load<Image>("sakura.png"), [this, i]() {
                log::info("Clicked me: {}", i);
                push_screen<PlayerScreen>(i);
            });
        }
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
