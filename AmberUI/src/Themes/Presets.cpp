// Amber UI - Built-in theme presets
// SPDX-License-Identifier: MIT
#include "AmberUI/Theme.h"

#include <algorithm>
#include <cctype>

namespace AmberUI
{
namespace
{
void SetPalette(Theme& theme, const char* background, const char* panel, const char* secondary,
                const char* header, const char* frame, const char* frame_hovered, const char* frame_active,
                const char* border, const char* border_strong, const char* accent, const char* accent_hovered,
                const char* accent_active, const char* text, const char* text_secondary,
                const char* text_disabled, const char* text_muted)
{
    theme[ColorToken::Background] = HexToColor(background);
    theme[ColorToken::PanelBackground] = HexToColor(panel);
    theme[ColorToken::SecondaryBackground] = HexToColor(secondary);
    theme[ColorToken::HeaderBackground] = HexToColor(header);
    theme[ColorToken::FrameBackground] = HexToColor(frame);
    theme[ColorToken::FrameHovered] = HexToColor(frame_hovered);
    theme[ColorToken::FrameActive] = HexToColor(frame_active);
    theme[ColorToken::Border] = HexToColor(border);
    theme[ColorToken::BorderStrong] = HexToColor(border_strong);
    theme[ColorToken::Accent] = HexToColor(accent);
    theme[ColorToken::AccentHovered] = HexToColor(accent_hovered);
    theme[ColorToken::AccentActive] = HexToColor(accent_active);
    theme[ColorToken::TextPrimary] = HexToColor(text);
    theme[ColorToken::TextSecondary] = HexToColor(text_secondary);
    theme[ColorToken::TextDisabled] = HexToColor(text_disabled);
    theme[ColorToken::TextMuted] = HexToColor(text_muted);
}

void ApplyCommonFonts(Theme& theme)
{
    // Relative paths are resolved by the Typography loader against
    // AMBERUI_ASSETS_DIR and the executable directory. Missing files fall back
    // to ImGui's built-in font.
    theme.fonts.regular = "fonts/Inter-Regular.ttf";
    theme.fonts.medium = "fonts/Inter-Medium.ttf";
    theme.fonts.bold = "fonts/Inter-SemiBold.ttf";
    theme.fonts.mono = "fonts/JetBrainsMono-Regular.ttf";
}
} // namespace

Theme AmberDark()
{
    Theme theme;
    theme.name = "Amber Dark";
    SetPalette(theme, "#181818", "#222222", "#262626", "#2B2B2B", "#292929", "#363636", "#404040", "#353535",
               "#484848", "#C4A36B", "#DFBE84", "#AA864F", "#D6D6D6", "#A6A6A6", "#686868", "#858585");
    ApplyCommonFonts(theme);
    return theme;
}

Theme AmberGraphite()
{
    Theme theme = AmberDark();
    theme.name = "Amber Graphite";
    SetPalette(theme, "#202020", "#292929", "#2E2E2E", "#333333", "#303030", "#3B3B3B", "#454545", "#3A3A3A",
               "#505050", "#B9A17C", "#D2BC96", "#9C8560", "#DADDDF", "#A9ACAE", "#6B6E70", "#888B8D");
    theme[ColorToken::Success] = HexToColor("#6F9B72");
    theme[ColorToken::Warning] = HexToColor("#CCAA63");
    theme[ColorToken::Error] = HexToColor("#B85E5E");
    theme[ColorToken::Info] = HexToColor("#5E97B5");
    return theme;
}

Theme AmberMidnight()
{
    Theme theme = AmberDark();
    theme.name = "Amber Midnight";
    SetPalette(theme, "#14171C", "#1C2027", "#20252D", "#252B34", "#232830", "#2F3640", "#39414D", "#2E3540",
               "#424B59", "#C7A76E", "#E3C48B", "#AD8B55", "#D8DCE3", "#A3AAB5", "#656C77", "#838A95");
    theme[ColorToken::Success] = HexToColor("#6C9B7C");
    theme[ColorToken::Warning] = HexToColor("#D2AC62");
    theme[ColorToken::Error] = HexToColor("#BC6262");
    theme[ColorToken::Info] = HexToColor("#5F96C0");
    return theme;
}

Theme AmberOLED()
{
    Theme theme = AmberDark();
    theme.name = "Amber OLED";
    SetPalette(theme, "#000000", "#0A0A0A", "#0E0E0E", "#141414", "#121212", "#1E1E1E", "#282828", "#242424",
               "#383838", "#D0AE72", "#EBCB90", "#B49255", "#DEDEDE", "#A4A4A4", "#5C5C5C", "#7C7C7C");
    theme.metrics.windowRounding = 2.0f;
    theme.metrics.popupRounding = 3.0f;
    theme[ColorToken::Border] = HexToColor("#1E1E1E");
    theme[ColorToken::BorderStrong] = HexToColor("#3A3A3A");
    return theme;
}

Theme AmberLight()
{
    Theme theme = AmberDark();
    theme.name = "Amber Light";
    SetPalette(theme, "#E9E9E7", "#F2F2F0", "#EDEDEB", "#E2E2DF", "#FFFFFF", "#F7F7F5", "#EAEAE7", "#D5D5D1",
               "#BEBEB9", "#8A6A2F", "#A68345", "#6F521F", "#262624", "#56564F", "#9A9A93", "#7A7A72");
    theme[ColorToken::Success] = HexToColor("#4F7F54");
    theme[ColorToken::Warning] = HexToColor("#9A7420");
    theme[ColorToken::Error] = HexToColor("#A84444");
    theme[ColorToken::Info] = HexToColor("#3E6E8E");
    theme[ColorToken::AxisX] = HexToColor("#9E3F3F");
    theme[ColorToken::AxisY] = HexToColor("#4A7040");
    theme[ColorToken::AxisZ] = HexToColor("#3A6390");
    theme.metrics.windowBorderSize = 1.0f;
    theme.metrics.frameBorderSize = 1.0f;
    return theme;
}

Theme AmberHighContrast()
{
    Theme theme = AmberDark();
    theme.name = "Amber High Contrast";
    SetPalette(theme, "#0B0B0B", "#141414", "#191919", "#202020", "#1B1B1B", "#2C2C2C", "#383838", "#5B5B5B",
               "#8C8C8C", "#F0C66F", "#FFDA8E", "#CFA54F", "#F5F5F5", "#D0D0D0", "#8A8A8A", "#A8A8A8");
    theme[ColorToken::Border] = HexToColor("#6A6A6A");
    theme[ColorToken::BorderStrong] = HexToColor("#9A9A9A");
    theme.metrics.windowBorderSize = 1.0f;
    theme.metrics.childBorderSize = 1.0f;
    theme.metrics.frameBorderSize = 1.0f;
    return theme;
}

Theme UE5Dark()
{
    Theme theme = AmberDark();
    theme.name = "UE5 Dark";
    SetPalette(theme, "#151515", "#242424", "#383838", "#2F2F2F", "#0F0F0F", "#575757", "#1A1A1A", "#383838",
               "#4C4C4C", "#0070E0", "#0E86FF", "#0050A0", "#C0C0C0", "#9E9E9E", "#6A6A6A", "#808080");
    theme[ColorToken::Success] = HexToColor("#1FE44B");
    theme[ColorToken::Warning] = HexToColor("#FFB800");
    theme[ColorToken::Error] = HexToColor("#EF3535");
    theme[ColorToken::Info] = HexToColor("#26BBFF");
    theme[ColorToken::AxisX] = HexToColor("#FF4040");
    theme[ColorToken::AxisY] = HexToColor("#8BC24A");
    theme[ColorToken::AxisZ] = HexToColor("#26BBFF");

    theme.metrics.windowPadding = ImVec2(6.0f, 6.0f);
    theme.metrics.itemSpacing = ImVec2(8.0f, 4.0f);
    theme.metrics.selectableTextAlign = ImVec2(0.0f, 0.5f);

    theme.metrics.windowRounding = 0.0f;
    theme.metrics.childRounding = 0.0f;
    theme.metrics.frameRounding = 4.0f;
    theme.metrics.popupRounding = 2.0f;
    theme.metrics.scrollbarRounding = 6.0f;
    theme.metrics.grabRounding = 5.0f;
    theme.metrics.tabRounding = 0.0f;

    theme.metrics.frameBorderSize = 1.0f;
    theme.metrics.scrollbarSize = 12.0f;
    theme.metrics.grabMinSize = 10.0f;

    theme.metrics.fontSizeBase = 13.0f;
    theme.metrics.fontSizeSmall = 11.0f;
    theme.metrics.fontSizeTitle = 15.0f;
    theme.metrics.fontSizeMono = 12.0f;

    theme.metrics.rowHeight = 21.0f;
    theme.metrics.controlHeight = 21.0f;
    return theme;
}

std::vector<Theme> BuiltInThemes()
{
    return { AmberDark(), AmberGraphite(), AmberMidnight(), AmberOLED(), AmberLight(), AmberHighContrast(),
             UE5Dark() };
}

std::string ThemeDisplayName(const std::string& theme_id)
{
    if (theme_id.empty())
        return std::string();
    char buffer[128];
    size_t out = 0;
    for (size_t i = 0; i < theme_id.size() && out + 2 < sizeof(buffer); ++i)
    {
        const char c = theme_id[i];
        if (i > 0 && std::isupper(static_cast<unsigned char>(c)))
            buffer[out++] = ' ';
        buffer[out++] = c;
    }
    buffer[out] = '\0';
    return std::string(buffer);
}

} // namespace AmberUI
