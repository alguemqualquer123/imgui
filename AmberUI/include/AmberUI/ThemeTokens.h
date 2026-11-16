// Amber UI - Design tokens
// SPDX-License-Identifier: MIT
#pragma once

#include <imgui.h>

#include <array>
#include <string>

namespace AmberUI
{
// -----------------------------------------------------------------------------
// Semantic color tokens. No widget should hardcode a color: everything resolves
// through this palette so a theme swap affects the whole library.
// -----------------------------------------------------------------------------
enum class ColorToken
{
    Background,
    PanelBackground,
    SecondaryBackground,
    HeaderBackground,
    FrameBackground,
    FrameHovered,
    FrameActive,
    Border,
    BorderStrong,

    Accent,
    AccentHovered,
    AccentActive,

    TextPrimary,
    TextSecondary,
    TextDisabled,
    TextMuted,

    Success,
    Warning,
    Error,
    Info,

    AxisX,
    AxisY,
    AxisZ,

    Count
};

constexpr int kColorTokenCount = static_cast<int>(ColorToken::Count);

// Stable machine name (used by the JSON theme format). e.g. "AccentHovered".
const char* ColorTokenName(ColorToken token);

// Resolve a token from its machine name. Returns false when unknown.
bool ColorTokenFromName(const char* name, ColorToken& out_token);

// Human readable label for the theme editor.
const char* ColorTokenLabel(ColorToken token);

// -----------------------------------------------------------------------------
// Layout metrics, expressed in unscaled pixels. They are multiplied by the
// theme DPI scale when applied to ImGuiStyle.
// -----------------------------------------------------------------------------
struct ThemeMetrics
{
    ImVec2 windowPadding      = ImVec2(8.0f, 8.0f);
    ImVec2 framePadding       = ImVec2(6.0f, 4.0f);
    ImVec2 itemSpacing        = ImVec2(6.0f, 4.0f);
    ImVec2 itemInnerSpacing   = ImVec2(4.0f, 4.0f);
    ImVec2 cellPadding        = ImVec2(4.0f, 2.0f);
    ImVec2 windowTitleAlign   = ImVec2(0.0f, 0.5f);
    ImVec2 buttonTextAlign    = ImVec2(0.5f, 0.5f);
    ImVec2 selectableTextAlign= ImVec2(0.0f, 0.0f);

    float  windowRounding     = 3.0f;
    float  childRounding      = 3.0f;
    float  frameRounding      = 3.0f;
    float  popupRounding      = 4.0f;
    float  scrollbarRounding  = 3.0f;
    float  grabRounding       = 2.0f;
    float  tabRounding        = 2.0f;

    float  windowBorderSize   = 1.0f;
    float  childBorderSize    = 1.0f;
    float  frameBorderSize    = 0.0f;
    float  popupBorderSize    = 1.0f;
    float  tabBorderSize      = 0.0f;

    float  scrollbarSize      = 11.0f;
    float  grabMinSize        = 9.0f;
    float  indentSpacing      = 16.0f;
    float  separatorTextBorder= 1.0f;

    float  windowMinWidth     = 220.0f;
    float  windowMinHeight    = 120.0f;

    // Base unscaled UI font size in pixels (before DPI/font scaling).
    float  fontSizeBase       = 14.0f;
    float  fontSizeSmall      = 12.0f;
    float  fontSizeTitle      = 16.0f;
    float  fontSizeMono       = 13.0f;

    // Component metrics used by Amber UI widgets (still theme owned).
    float  rowHeight          = 22.0f;
    float  toolbarHeight      = 36.0f;
    float  statusBarHeight    = 24.0f;
    float  controlHeight      = 24.0f;
};

// -----------------------------------------------------------------------------
// Font configuration. Empty paths mean "keep current ImGui font".
// -----------------------------------------------------------------------------
enum class FontRole
{
    Regular,
    Medium,
    Bold,
    Mono,
    Count
};

struct ThemeFonts
{
    std::string regular;
    std::string medium;
    std::string bold;
    std::string mono;

    const std::string& Path(FontRole role) const;
    std::string& Path(FontRole role);
};

// -----------------------------------------------------------------------------
// A complete theme: palette + metrics + fonts + transparency + DPI.
// -----------------------------------------------------------------------------
struct Theme
{
    std::string name = "Amber Dark";
    std::array<ImVec4, kColorTokenCount> colors{};
    ThemeMetrics metrics{};
    ThemeFonts fonts{};
    float alpha    = 1.0f; // global ImGuiStyle::Alpha multiplier
    float dpiScale = 1.0f; // additional metric scaling (user/DPI owned)

    Theme();

    ImVec4& Color(ColorToken token) { return colors[static_cast<int>(token)]; }
    const ImVec4& Color(ColorToken token) const { return colors[static_cast<int>(token)]; }

    ImVec4& operator[](ColorToken token) { return Color(token); }
    const ImVec4& operator[](ColorToken token) const { return Color(token); }

    // Effective scale applied to metrics (theme DPI only; fonts use their own).
    float MetricScale() const { return dpiScale > 0.0f ? dpiScale : 1.0f; }

    void SetDpiScale(float scale);
    float GetDpiScale() const { return dpiScale; }
};

// -----------------------------------------------------------------------------
// Color helpers (sRGB, no gamma conversion; ImGui works in linear 0..1 values
// that are simply components of the authored sRGB colors).
// -----------------------------------------------------------------------------
ImVec4 HexToColor(const char* hex, float alpha = 1.0f); // accepts "#RRGGBB" / "RRGGBBAA"
std::string ColorToHex(const ImVec4& color, bool with_alpha = false);
ImVec4 WithAlpha(const ImVec4& color, float alpha);
ImVec4 LerpColor(const ImVec4& a, const ImVec4& b, float t);
ImVec4 MultiplyColor(const ImVec4& color, float factor);
ImVec4 Lighten(const ImVec4& color, float amount);
ImVec4 Darken(const ImVec4& color, float amount);

// Deterministic small palette helper used by graphs/timelines/assets.
ImVec4 CategoryColor(const char* category);

} // namespace AmberUI
