#pragma once

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

namespace toolkit {

struct Color {
    float r, g, b, a;

    // From RGBA (uint8_t)
    static Color from_rgba_u8(
        std::uint8_t r,
        std::uint8_t g,
        std::uint8_t b,
        std::uint8_t a
    ) {
        return Color {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    }

    // From RGB (uint8_t)
    static Color from_rgb_u8(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
        return from_rgba_u8(r, g, b, 255);
    }

    // From RGBA (float)
    static Color from_rgba(float r, float g, float b, float a) {
        return Color {r, g, b, a};
    }

    // From RGB (float)
    static Color from_rgb(float r, float g, float b) {
        return from_rgba(r, g, b, 1.0f);
    }

    // From Hex String (e.g., "#RRGGBB" or "#RRGGBBAA")
    // Supports #RGB, #rgb, #RGBA, #rgba, #RRGGBB, #rrggbb, #RRGGBBAA and
    // #rrggbbaa
    static Color from_hex(const std::string& hex) {
        if (hex.empty() || hex[0] != '#') {
            throw std::invalid_argument("Invalid hex color format.");
        }

        std::string hex_value = hex.substr(1);  // Remove '#'

        // Convert to uppercase for consistency
        std::transform(
            hex_value.begin(),
            hex_value.end(),
            hex_value.begin(),
            [](unsigned char c) { return std::toupper(c); }
        );

        // Check the length
        if (hex_value.length() != 3 && hex_value.length() != 4
            && hex_value.length() != 6 && hex_value.length() != 8) {
            throw std::invalid_argument("Invalid hex color length.");
        }

        // Expand 3 or 4 character hex
        if (hex_value.length() == 3 || hex_value.length() == 4) {
            std::string expanded;
            for (char c : hex_value) {
                expanded += c;
                expanded += c;
            }
            hex_value = expanded;
        }

        // Now hexValue is either 6 or 8 characters long
        std::uint32_t value;
        std::stringstream ss;
        ss << std::hex << hex_value;
        ss >> value;

        if (hex_value.length() == 6) {
            // If the hex code is 6 digits, assume full opacity (1.0)
            return from_rgba(
                static_cast<float>((value >> 16) & 0xFF) / 255.0f,
                static_cast<float>((value >> 8) & 0xFF) / 255.0f,
                static_cast<float>(value & 0xFF) / 255.0f,
                1.0f
            );
        } else {
            // If the hex code is 8 digits, parse the alpha channel
            return from_rgba(
                static_cast<float>((value >> 24) & 0xFF) / 255.0f,
                static_cast<float>((value >> 16) & 0xFF) / 255.0f,
                static_cast<float>((value >> 8) & 0xFF) / 255.0f,
                static_cast<float>(value & 0xFF) / 255.0f
            );
        }
    }

    Color set_a(float new_a) const {
        return Color::from_rgba(r, g, b, new_a);
    }

    static const Color RED;
    static const Color CRIMSON;
    static const Color DARK_RED;
    static const Color FIREBRICK;
    static const Color INDIAN_RED;
    static const Color LIGHT_CORAL;
    static const Color SALMON;
    static const Color DARK_SALMON;
    static const Color LIGHT_SALMON;
    static const Color ORANGE_RED;
    static const Color TOMATO;
    static const Color DARK_ORANGE;
    static const Color ORANGE;
    static const Color GOLD;
    static const Color DARK_GOLDEN_ROD;
    static const Color GOLDEN_ROD;
    static const Color PALE_GOLDEN_ROD;
    static const Color DARK_KHAKI;
    static const Color KHAKI;
    static const Color OLIVE;
    static const Color YELLOW;
    static const Color YELLOW_GREEN;
    static const Color DARK_OLIVE_GREEN;
    static const Color OLIVE_DRAB;
    static const Color LAWN_GREEN;
    static const Color CHART_REUSE;
    static const Color GREEN_YELLOW;
    static const Color DARK_GREEN;
    static const Color GREEN;
    static const Color FOREST_GREEN;
    static const Color LIME;
    static const Color LIME_GREEN;
    static const Color LIGHT_GREEN;
    static const Color PALE_GREEN;
    static const Color DARK_SEA_GREEN;
    static const Color MEDIUM_SPRING_GREEN;
    static const Color SPRING_GREEN;
    static const Color SEA_GREEN;
    static const Color MEDIUM_AQUA_MARINE;
    static const Color MEDIUM_SEA_GREEN;
    static const Color LIGHT_SEA_GREEN;
    static const Color DARK_SLATE_GRAY;
    static const Color TEAL;
    static const Color DARK_CYAN;
    static const Color AQUA;
    static const Color CYAN;
    static const Color LIGHT_CYAN;
    static const Color DARK_TURQUOISE;
    static const Color TURQUOISE;
    static const Color MEDIUM_TURQUOISE;
    static const Color PALE_TURQUOISE;
    static const Color AQUA_MARINE;
    static const Color POWDER_BLUE;
    static const Color CADET_BLUE;
    static const Color STEEL_BLUE;
    static const Color CORNFLOWER_BLUE;
    static const Color DEEP_SKY_BLUE;
    static const Color DODGER_BLUE;
    static const Color LIGHT_BLUE;
    static const Color SKY_BLUE;
    static const Color LIGHT_SKY_BLUE;
    static const Color MIDNIGHT_BLUE;
    static const Color NAVY;
    static const Color DARK_BLUE;
    static const Color MEDIUM_BLUE;
    static const Color BLUE;
    static const Color ROYAL_BLUE;
    static const Color BLUE_VIOLET;
    static const Color INDIGO;
    static const Color DARK_SLATE_BLUE;
    static const Color SLATE_BLUE;
    static const Color MEDIUM_SLATE_BLUE;
    static const Color MEDIUM_PURPLE;
    static const Color DARK_MAGENTA;
    static const Color DARK_VIOLET;
    static const Color DARK_ORCHID;
    static const Color MEDIUM_ORCHID;
    static const Color PURPLE;
    static const Color THISTLE;
    static const Color PLUM;
    static const Color VIOLET;
    static const Color MAGENTA;
    static const Color ORCHID;
    static const Color MEDIUM_VIOLET_RED;
    static const Color PALE_VIOLET_RED;
    static const Color DEEP_PINK;
    static const Color HOT_PINK;
    static const Color LIGHT_PINK;
    static const Color PINK;
    static const Color ANTIQUE_WHITE;
    static const Color BEIGE;
    static const Color BISQUE;
    static const Color BLANCHED_ALMOND;
    static const Color WHEAT;
    static const Color CORN_SILK;
    static const Color LEMON_CHIFFON;
    static const Color LIGHT_GOLDEN_ROD_YELLOW;
    static const Color LIGHT_YELLOW;
    static const Color SADDLE_BROWN;
    static const Color SIENNA;
    static const Color CHOCOLATE;
    static const Color PERU;
    static const Color SANDY_BROWN;
    static const Color BURLY_WOOD;
    static const Color TAN;
    static const Color ROSY_BROWN;
    static const Color MOCCASIN;
    static const Color NAVAJO_WHITE;
    static const Color PEACH_PUFF;
    static const Color MISTY_ROSE;
    static const Color LAVENDER_BLUSH;
    static const Color LINEN;
    static const Color OLD_LACE;
    static const Color PAPAYA_WHIP;
    static const Color SEA_SHELL;
    static const Color MINT_CREAM;
    static const Color SLATE_GRAY;
    static const Color LIGHT_SLATE_GRAY;
    static const Color LIGHT_STEEL_BLUE;
    static const Color LAVENDER;
    static const Color FLORAL_WHITE;
    static const Color ALICE_BLUE;
    static const Color GHOST_WHITE;
    static const Color HONEYDEW;
    static const Color IVORY;
    static const Color AZURE;
    static const Color SNOW;
    static const Color BLACK;
    static const Color DIM_GRAY;
    static const Color GRAY;
    static const Color DARK_GRAY;
    static const Color SILVER;
    static const Color LIGHT_GRAY;
    static const Color GAINSBORO;
    static const Color WHITE_SMOKE;
    static const Color WHITE;
#ifdef TRANSPARENT
    #undef TRANSPARENT
#endif
    static const Color TRANSPARENT;
};

inline const Color Color::RED = Color::from_rgb_u8(255, 0, 0);
inline const Color Color::CRIMSON = Color::from_rgb_u8(220, 20, 60);
inline const Color Color::DARK_RED = Color::from_rgb_u8(139, 0, 0);
inline const Color Color::FIREBRICK = Color::from_rgb_u8(178, 34, 34);
inline const Color Color::INDIAN_RED = Color::from_rgb_u8(205, 92, 92);
inline const Color Color::LIGHT_CORAL = Color::from_rgb_u8(240, 128, 128);
inline const Color Color::SALMON = Color::from_rgb_u8(250, 128, 114);
inline const Color Color::DARK_SALMON = Color::from_rgb_u8(233, 150, 122);
inline const Color Color::LIGHT_SALMON = Color::from_rgb_u8(255, 160, 122);
inline const Color Color::ORANGE_RED = Color::from_rgb_u8(255, 69, 0);
inline const Color Color::TOMATO = Color::from_rgb_u8(255, 99, 71);
inline const Color Color::DARK_ORANGE = Color::from_rgb_u8(255, 140, 0);
inline const Color Color::ORANGE = Color::from_rgb_u8(255, 165, 0);
inline const Color Color::GOLD = Color::from_rgb_u8(255, 215, 0);
inline const Color Color::DARK_GOLDEN_ROD = Color::from_rgb_u8(184, 134, 11);
inline const Color Color::GOLDEN_ROD = Color::from_rgb_u8(218, 165, 32);
inline const Color Color::PALE_GOLDEN_ROD = Color::from_rgb_u8(238, 232, 170);
inline const Color Color::DARK_KHAKI = Color::from_rgb_u8(189, 183, 107);
inline const Color Color::KHAKI = Color::from_rgb_u8(240, 230, 140);
inline const Color Color::OLIVE = Color::from_rgb_u8(128, 128, 0);
inline const Color Color::YELLOW = Color::from_rgb_u8(255, 255, 0);
inline const Color Color::YELLOW_GREEN = Color::from_rgb_u8(154, 205, 50);
inline const Color Color::DARK_OLIVE_GREEN = Color::from_rgb_u8(85, 107, 47);
inline const Color Color::OLIVE_DRAB = Color::from_rgb_u8(107, 142, 35);
inline const Color Color::LAWN_GREEN = Color::from_rgb_u8(124, 252, 0);
inline const Color Color::CHART_REUSE = Color::from_rgb_u8(127, 255, 0);
inline const Color Color::GREEN_YELLOW = Color::from_rgb_u8(173, 255, 47);
inline const Color Color::DARK_GREEN = Color::from_rgb_u8(0, 100, 0);
inline const Color Color::GREEN = Color::from_rgb_u8(0, 128, 0);
inline const Color Color::FOREST_GREEN = Color::from_rgb_u8(34, 139, 34);
inline const Color Color::LIME = Color::from_rgb_u8(0, 255, 0);
inline const Color Color::LIME_GREEN = Color::from_rgb_u8(50, 205, 50);
inline const Color Color::LIGHT_GREEN = Color::from_rgb_u8(144, 238, 144);
inline const Color Color::PALE_GREEN = Color::from_rgb_u8(152, 251, 152);
inline const Color Color::DARK_SEA_GREEN = Color::from_rgb_u8(143, 188, 143);
inline const Color Color::MEDIUM_SPRING_GREEN = Color::from_rgb_u8(0, 250, 154);
inline const Color Color::SPRING_GREEN = Color::from_rgb_u8(0, 255, 127);
inline const Color Color::SEA_GREEN = Color::from_rgb_u8(46, 139, 87);
inline const Color Color::MEDIUM_AQUA_MARINE =
    Color::from_rgb_u8(102, 205, 170);
inline const Color Color::MEDIUM_SEA_GREEN = Color::from_rgb_u8(60, 179, 113);
inline const Color Color::LIGHT_SEA_GREEN = Color::from_rgb_u8(32, 178, 170);
inline const Color Color::DARK_SLATE_GRAY = Color::from_rgb_u8(47, 79, 79);
inline const Color Color::TEAL = Color::from_rgb_u8(0, 128, 128);
inline const Color Color::DARK_CYAN = Color::from_rgb_u8(0, 139, 139);
inline const Color Color::AQUA = Color::from_rgb_u8(0, 255, 255);
inline const Color Color::CYAN = Color::from_rgb_u8(0, 255, 255);
inline const Color Color::LIGHT_CYAN = Color::from_rgb_u8(224, 255, 255);
inline const Color Color::DARK_TURQUOISE = Color::from_rgb_u8(0, 206, 209);
inline const Color Color::TURQUOISE = Color::from_rgb_u8(64, 224, 208);
inline const Color Color::MEDIUM_TURQUOISE = Color::from_rgb_u8(72, 209, 204);
inline const Color Color::PALE_TURQUOISE = Color::from_rgb_u8(175, 238, 238);
inline const Color Color::AQUA_MARINE = Color::from_rgb_u8(127, 255, 212);
inline const Color Color::POWDER_BLUE = Color::from_rgb_u8(176, 224, 230);
inline const Color Color::CADET_BLUE = Color::from_rgb_u8(95, 158, 160);
inline const Color Color::STEEL_BLUE = Color::from_rgb_u8(70, 130, 180);
inline const Color Color::CORNFLOWER_BLUE = Color::from_rgb_u8(100, 149, 237);
inline const Color Color::DEEP_SKY_BLUE = Color::from_rgb_u8(0, 191, 255);
inline const Color Color::DODGER_BLUE = Color::from_rgb_u8(30, 144, 255);
inline const Color Color::LIGHT_BLUE = Color::from_rgb_u8(173, 216, 230);
inline const Color Color::SKY_BLUE = Color::from_rgb_u8(135, 206, 235);
inline const Color Color::LIGHT_SKY_BLUE = Color::from_rgb_u8(135, 206, 250);
inline const Color Color::MIDNIGHT_BLUE = Color::from_rgb_u8(25, 25, 112);
inline const Color Color::NAVY = Color::from_rgb_u8(0, 0, 128);
inline const Color Color::DARK_BLUE = Color::from_rgb_u8(0, 0, 139);
inline const Color Color::MEDIUM_BLUE = Color::from_rgb_u8(0, 0, 205);
inline const Color Color::BLUE = Color::from_rgb_u8(0, 0, 255);
inline const Color Color::ROYAL_BLUE = Color::from_rgb_u8(65, 105, 225);
inline const Color Color::BLUE_VIOLET = Color::from_rgb_u8(138, 43, 226);
inline const Color Color::INDIGO = Color::from_rgb_u8(75, 0, 130);
inline const Color Color::DARK_SLATE_BLUE = Color::from_rgb_u8(72, 61, 139);
inline const Color Color::SLATE_BLUE = Color::from_rgb_u8(106, 90, 205);
inline const Color Color::MEDIUM_SLATE_BLUE = Color::from_rgb_u8(123, 104, 238);
inline const Color Color::MEDIUM_PURPLE = Color::from_rgb_u8(147, 112, 219);
inline const Color Color::DARK_MAGENTA = Color::from_rgb_u8(139, 0, 139);
inline const Color Color::DARK_VIOLET = Color::from_rgb_u8(148, 0, 211);
inline const Color Color::DARK_ORCHID = Color::from_rgb_u8(153, 50, 204);
inline const Color Color::MEDIUM_ORCHID = Color::from_rgb_u8(186, 85, 211);
inline const Color Color::PURPLE = Color::from_rgb_u8(128, 0, 128);
inline const Color Color::THISTLE = Color::from_rgb_u8(216, 191, 216);
inline const Color Color::PLUM = Color::from_rgb_u8(221, 160, 221);
inline const Color Color::VIOLET = Color::from_rgb_u8(238, 130, 238);
inline const Color Color::MAGENTA = Color::from_rgb_u8(255, 0, 255);
inline const Color Color::ORCHID = Color::from_rgb_u8(218, 112, 214);
inline const Color Color::MEDIUM_VIOLET_RED = Color::from_rgb_u8(199, 21, 133);
inline const Color Color::PALE_VIOLET_RED = Color::from_rgb_u8(219, 112, 147);
inline const Color Color::DEEP_PINK = Color::from_rgb_u8(255, 20, 147);
inline const Color Color::HOT_PINK = Color::from_rgb_u8(255, 105, 180);
inline const Color Color::LIGHT_PINK = Color::from_rgb_u8(255, 182, 193);
inline const Color Color::PINK = Color::from_rgb_u8(255, 192, 203);
inline const Color Color::ANTIQUE_WHITE = Color::from_rgb_u8(250, 235, 215);
inline const Color Color::BEIGE = Color::from_rgb_u8(245, 245, 220);
inline const Color Color::BISQUE = Color::from_rgb_u8(255, 228, 196);
inline const Color Color::BLANCHED_ALMOND = Color::from_rgb_u8(255, 235, 205);
inline const Color Color::WHEAT = Color::from_rgb_u8(245, 222, 179);
inline const Color Color::CORN_SILK = Color::from_rgb_u8(255, 248, 220);
inline const Color Color::LEMON_CHIFFON = Color::from_rgb_u8(255, 250, 205);
inline const Color Color::LIGHT_GOLDEN_ROD_YELLOW =
    Color::from_rgb_u8(250, 250, 210);
inline const Color Color::LIGHT_YELLOW = Color::from_rgb_u8(255, 255, 224);
inline const Color Color::SADDLE_BROWN = Color::from_rgb_u8(139, 69, 19);
inline const Color Color::SIENNA = Color::from_rgb_u8(160, 82, 45);
inline const Color Color::CHOCOLATE = Color::from_rgb_u8(210, 105, 30);
inline const Color Color::PERU = Color::from_rgb_u8(205, 133, 63);
inline const Color Color::SANDY_BROWN = Color::from_rgb_u8(244, 164, 96);
inline const Color Color::BURLY_WOOD = Color::from_rgb_u8(222, 184, 135);
inline const Color Color::TAN = Color::from_rgb_u8(210, 180, 140);
inline const Color Color::ROSY_BROWN = Color::from_rgb_u8(188, 143, 143);
inline const Color Color::MOCCASIN = Color::from_rgb_u8(255, 228, 181);
inline const Color Color::NAVAJO_WHITE = Color::from_rgb_u8(255, 222, 173);
inline const Color Color::PEACH_PUFF = Color::from_rgb_u8(255, 218, 185);
inline const Color Color::MISTY_ROSE = Color::from_rgb_u8(255, 228, 225);
inline const Color Color::LAVENDER_BLUSH = Color::from_rgb_u8(255, 240, 245);
inline const Color Color::LINEN = Color::from_rgb_u8(250, 240, 230);
inline const Color Color::OLD_LACE = Color::from_rgb_u8(253, 245, 230);
inline const Color Color::PAPAYA_WHIP = Color::from_rgb_u8(255, 239, 213);
inline const Color Color::SEA_SHELL = Color::from_rgb_u8(255, 245, 238);
inline const Color Color::MINT_CREAM = Color::from_rgb_u8(245, 255, 250);
inline const Color Color::SLATE_GRAY = Color::from_rgb_u8(112, 128, 144);
inline const Color Color::LIGHT_SLATE_GRAY = Color::from_rgb_u8(119, 136, 153);
inline const Color Color::LIGHT_STEEL_BLUE = Color::from_rgb_u8(176, 196, 222);
inline const Color Color::LAVENDER = Color::from_rgb_u8(230, 230, 250);
inline const Color Color::FLORAL_WHITE = Color::from_rgb_u8(255, 250, 240);
inline const Color Color::ALICE_BLUE = Color::from_rgb_u8(240, 248, 255);
inline const Color Color::GHOST_WHITE = Color::from_rgb_u8(248, 248, 255);
inline const Color Color::HONEYDEW = Color::from_rgb_u8(240, 255, 240);
inline const Color Color::IVORY = Color::from_rgb_u8(255, 255, 240);
inline const Color Color::AZURE = Color::from_rgb_u8(240, 255, 255);
inline const Color Color::SNOW = Color::from_rgb_u8(255, 250, 250);
inline const Color Color::BLACK = Color::from_rgb_u8(0, 0, 0);
inline const Color Color::DIM_GRAY = Color::from_rgb_u8(105, 105, 105);
inline const Color Color::GRAY = Color::from_rgb_u8(128, 128, 128);
inline const Color Color::DARK_GRAY = Color::from_rgb_u8(169, 169, 169);
inline const Color Color::SILVER = Color::from_rgb_u8(192, 192, 192);
inline const Color Color::LIGHT_GRAY = Color::from_rgb_u8(211, 211, 211);
inline const Color Color::GAINSBORO = Color::from_rgb_u8(220, 220, 220);
inline const Color Color::WHITE_SMOKE = Color::from_rgb_u8(245, 245, 245);
inline const Color Color::WHITE = Color::from_rgb_u8(255, 255, 255);
inline const Color Color::TRANSPARENT = Color::from_rgba(0.f, 0.f, 0.f, 0.f);

}  // namespace toolkit
