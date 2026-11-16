// Amber UI - Design token implementation
// SPDX-License-Identifier: MIT
#include "AmberUI/ThemeTokens.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

namespace AmberUI
{
namespace
{
struct TokenInfo
{
    ColorToken token;
    const char* name;
    const char* label;
};

const TokenInfo kTokens[] = {
    { ColorToken::Background, "Background", "Background" },
    { ColorToken::PanelBackground, "PanelBackground", "Panel Background" },
    { ColorToken::SecondaryBackground, "SecondaryBackground", "Secondary Background" },
    { ColorToken::HeaderBackground, "HeaderBackground", "Header Background" },
    { ColorToken::FrameBackground, "FrameBackground", "Frame Background" },
    { ColorToken::FrameHovered, "FrameHovered", "Frame Hovered" },
    { ColorToken::FrameActive, "FrameActive", "Frame Active" },
    { ColorToken::Border, "Border", "Border" },
    { ColorToken::BorderStrong, "BorderStrong", "Border Strong" },
    { ColorToken::Accent, "Accent", "Accent" },
    { ColorToken::AccentHovered, "AccentHovered", "Accent Hovered" },
    { ColorToken::AccentActive, "AccentActive", "Accent Active" },
    { ColorToken::TextPrimary, "TextPrimary", "Text Primary" },
    { ColorToken::TextSecondary, "TextSecondary", "Text Secondary" },
    { ColorToken::TextDisabled, "TextDisabled", "Text Disabled" },
    { ColorToken::TextMuted, "TextMuted", "Text Muted" },
    { ColorToken::Success, "Success", "Success" },
    { ColorToken::Warning, "Warning", "Warning" },
    { ColorToken::Error, "Error", "Error" },
    { ColorToken::Info, "Info", "Info" },
    { ColorToken::AxisX, "AxisX", "Axis X" },
    { ColorToken::AxisY, "AxisY", "Axis Y" },
    { ColorToken::AxisZ, "AxisZ", "Axis Z" },
};

int ToInt(ColorToken token) { return static_cast<int>(token); }
} // namespace

const char* ColorTokenName(ColorToken token)
{
    const int index = ToInt(token);
    if (index < 0 || index >= kColorTokenCount)
        return "Unknown";
    return kTokens[index].name;
}

bool ColorTokenFromName(const char* name, ColorToken& out_token)
{
    if (!name)
        return false;
    for (const TokenInfo& info : kTokens)
    {
        if (std::strcmp(info.name, name) == 0)
        {
            out_token = info.token;
            return true;
        }
    }
    return false;
}

const char* ColorTokenLabel(ColorToken token)
{
    const int index = ToInt(token);
    if (index < 0 || index >= kColorTokenCount)
        return "Unknown";
    return kTokens[index].label;
}

const std::string& ThemeFonts::Path(FontRole role) const
{
    switch (role)
    {
    case FontRole::Regular: return regular;
    case FontRole::Medium: return medium;
    case FontRole::Bold: return bold;
    case FontRole::Mono: return mono;
    default: return regular;
    }
}

std::string& ThemeFonts::Path(FontRole role)
{
    switch (role)
    {
    case FontRole::Regular: return regular;
    case FontRole::Medium: return medium;
    case FontRole::Bold: return bold;
    case FontRole::Mono: return mono;
    default: return regular;
    }
}

Theme::Theme()
{
    colors.fill(ImVec4(0, 0, 0, 1));
    // Amber Dark defaults. Presets override this palette.
    Color(ColorToken::Background) = HexToColor("#181818");
    Color(ColorToken::PanelBackground) = HexToColor("#222222");
    Color(ColorToken::SecondaryBackground) = HexToColor("#262626");
    Color(ColorToken::HeaderBackground) = HexToColor("#2B2B2B");
    Color(ColorToken::FrameBackground) = HexToColor("#292929");
    Color(ColorToken::FrameHovered) = HexToColor("#363636");
    Color(ColorToken::FrameActive) = HexToColor("#404040");
    Color(ColorToken::Border) = HexToColor("#353535");
    Color(ColorToken::BorderStrong) = HexToColor("#484848");
    Color(ColorToken::Accent) = HexToColor("#C4A36B");
    Color(ColorToken::AccentHovered) = HexToColor("#DFBE84");
    Color(ColorToken::AccentActive) = HexToColor("#AA864F");
    Color(ColorToken::TextPrimary) = HexToColor("#D6D6D6");
    Color(ColorToken::TextSecondary) = HexToColor("#A6A6A6");
    Color(ColorToken::TextDisabled) = HexToColor("#686868");
    Color(ColorToken::TextMuted) = HexToColor("#858585");
    Color(ColorToken::Success) = HexToColor("#68976B");
    Color(ColorToken::Warning) = HexToColor("#D4AB58");
    Color(ColorToken::Error) = HexToColor("#BC5555");
    Color(ColorToken::Info) = HexToColor("#5896BB");
    Color(ColorToken::AxisX) = HexToColor("#AA4646");
    Color(ColorToken::AxisY) = HexToColor("#547E45");
    Color(ColorToken::AxisZ) = HexToColor("#426DA1");
}

void Theme::SetDpiScale(float scale)
{
    dpiScale = std::clamp(scale, 0.5f, 4.0f);
}

ImVec4 HexToColor(const char* hex, float alpha)
{
    if (!hex)
        return ImVec4(1, 1, 1, alpha);
    while (*hex == '#' || *hex == ' ')
        ++hex;
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    const size_t length = std::strlen(hex);
    if (length != 6 && length != 8)
        return ImVec4(1, 1, 1, alpha);
    int values[8] = { 0 };
    for (size_t i = 0; i < length; ++i)
    {
        values[i] = nibble(hex[i]);
        if (values[i] < 0)
            return ImVec4(1, 1, 1, alpha);
    }
    auto component = [&](size_t i) { return static_cast<float>(values[i] * 16 + values[i + 1]) / 255.0f; };
    ImVec4 color(component(0), component(2), component(4), length == 8 ? component(6) : alpha);
    if (length == 6)
        color.w = alpha;
    return color;
}

std::string ColorToHex(const ImVec4& color, bool with_alpha)
{
    auto to_byte = [](float value) {
        const float clamped = std::clamp(value, 0.0f, 1.0f);
        return static_cast<int>(std::lround(clamped * 255.0f));
    };
    char buffer[16];
    if (with_alpha)
        std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X%02X", to_byte(color.x), to_byte(color.y),
                      to_byte(color.z), to_byte(color.w));
    else
        std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", to_byte(color.x), to_byte(color.y), to_byte(color.z));
    return std::string(buffer);
}

ImVec4 WithAlpha(const ImVec4& color, float alpha)
{
    return ImVec4(color.x, color.y, color.z, alpha);
}

ImVec4 LerpColor(const ImVec4& a, const ImVec4& b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t,
                  a.w + (b.w - a.w) * t);
}

ImVec4 MultiplyColor(const ImVec4& color, float factor)
{
    return ImVec4(color.x * factor, color.y * factor, color.z * factor, color.w);
}

ImVec4 Lighten(const ImVec4& color, float amount)
{
    return ImVec4(std::min(1.0f, color.x + amount), std::min(1.0f, color.y + amount),
                  std::min(1.0f, color.z + amount), color.w);
}

ImVec4 Darken(const ImVec4& color, float amount)
{
    return ImVec4(std::max(0.0f, color.x - amount), std::max(0.0f, color.y - amount),
                  std::max(0.0f, color.z - amount), color.w);
}

ImVec4 CategoryColor(const char* category)
{
    if (!category)
        return HexToColor("#7A7A7A");
    auto equals = [category](const char* value) {
        if (!value)
            return false;
        const char* lhs = category;
        while (*lhs && *value)
        {
            if (std::tolower(static_cast<unsigned char>(*lhs)) !=
                std::tolower(static_cast<unsigned char>(*value)))
                return false;
            ++lhs;
            ++value;
        }
        return *lhs == '\0' && *value == '\0';
    };
    if (equals("flow")) return HexToColor("#6F7276");
    if (equals("function")) return HexToColor("#5F7E9E");
    if (equals("input")) return HexToColor("#5E875E");
    if (equals("output")) return HexToColor("#9E5C5C");
    if (equals("animation")) return HexToColor("#8A8A4F");
    if (equals("math")) return HexToColor("#82699C");
    if (equals("event")) return HexToColor("#B07B4E");
    if (equals("data")) return HexToColor("#4F9199");
    if (equals("material")) return HexToColor("#8A6E9E");
    if (equals("mesh")) return HexToColor("#5E87A0");
    if (equals("texture")) return HexToColor("#9E7B5C");
    if (equals("audio")) return HexToColor("#5F9E8D");
    if (equals("video")) return HexToColor("#9E5F86");
    if (equals("script")) return HexToColor("#7F9E5F");
    if (equals("scene")) return HexToColor("#5F72A0");
    return HexToColor("#7A7A7A");
}

} // namespace AmberUI
