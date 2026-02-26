#include <toolkit/app.h>

#include <print>

using namespace toolkit;

int main(int argc, char** argv) {
    return App().run(argc, argv).unwrap_or_else([](const auto& e) {
        std::println(
            stderr,
            "Application ran into a fatal problem: {}",
            e.message()
        );
        return 1;
    });
}