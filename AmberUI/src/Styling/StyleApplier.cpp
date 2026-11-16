// Amber UI - ImGui style application
// SPDX-License-Identifier: MIT
#include "AmberUI/Theme.h"

#include <imgui.h>

#include <algorithm>

namespace AmberUI
{
namespace
{
ImVec2 Scaled(const ImVec2& value, float scale)
{
    return ImVec2(value.x * scale, value.y * scale);
}

float Scaled(float value, float scale)
{
    return value * scale;
}
} // namespace

void ApplyThemeToStyle(ImGuiStyle& style, const Theme& theme)
{
    const float scale = theme.MetricScale();
    const auto& c = theme.colors;
    auto col = [&](ColorToken token) -> ImVec4& { return const_cast<ImVec4&>(c[static_cast<int>(token)]); };

    // ---------------------------------------------------------------------
    // Metrics
    // ---------------------------------------------------------------------
    style.WindowPadding = Scaled(theme.metrics.windowPadding, scale);
    style.FramePadding = Scaled(theme.metrics.framePadding, scale);
    style.ItemSpacing = Scaled(theme.metrics.itemSpacing, scale);
    style.ItemInnerSpacing = Scaled(theme.metrics.itemInnerSpacing, scale);
    style.CellPadding = Scaled(theme.metrics.cellPadding, scale);
    style.WindowTitleAlign = theme.metrics.windowTitleAlign;
    style.ButtonTextAlign = theme.metrics.buttonTextAlign;
    style.SelectableTextAlign = theme.metrics.selectableTextAlign;

    style.WindowRounding = Scaled(theme.metrics.windowRounding, scale);
    style.ChildRounding = Scaled(theme.metrics.childRounding, scale);
    style.FrameRounding = Scaled(theme.metrics.frameRounding, scale);
    style.PopupRounding = Scaled(theme.metrics.popupRounding, scale);
    style.ScrollbarRounding = Scaled(theme.metrics.scrollbarRounding, scale);
    style.GrabRounding = Scaled(theme.metrics.grabRounding, scale);
    style.TabRounding = Scaled(theme.metrics.tabRounding, scale);

    style.WindowBorderSize = Scaled(theme.metrics.windowBorderSize, scale);
    style.ChildBorderSize = Scaled(theme.metrics.childBorderSize, scale);
    style.FrameBorderSize = Scaled(theme.metrics.frameBorderSize, scale);
    style.PopupBorderSize = Scaled(theme.metrics.popupBorderSize, scale);
    style.TabBorderSize = Scaled(theme.metrics.tabBorderSize, scale);

    style.ScrollbarSize = Scaled(theme.metrics.scrollbarSize, scale);
    style.ScrollbarPadding = 1.0f * scale;
    style.GrabMinSize = Scaled(theme.metrics.grabMinSize, scale);
    style.IndentSpacing = Scaled(theme.metrics.indentSpacing, scale);
    style.SeparatorTextBorderSize = Scaled(theme.metrics.separatorTextBorder, scale);
    style.SeparatorTextPadding = ImVec2(12.0f * scale, 3.0f * scale);
    style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);

    style.WindowMinSize = ImVec2(Scaled(theme.metrics.windowMinWidth, scale), Scaled(theme.metrics.windowMinHeight, scale));
    style.WindowBorderHoverPadding = 4.0f * scale;
    style.WindowMenuButtonPosition = ImGuiDir_Left;

    style.Alpha = std::clamp(theme.alpha, 0.0f, 1.0f);
    style.DisabledAlpha = 0.55f;

    style.TabBarBorderSize = 1.0f * scale;
    style.TabBarOverlineSize = 2.0f * scale;
    style.DockingSeparatorSize = 1.0f * scale;
    style.DockingNodeHasCloseButton = true;

    style.TreeLinesSize = 1.0f * scale;
    style.TreeLinesRounding = 2.0f * scale;
    style.DragDropTargetBorderSize = 1.0f * scale;
    style.DragDropTargetRounding = 2.0f * scale;
    style.DragDropTargetPadding = 2.0f * scale;
    style.ColorMarkerSize = 10.0f * scale;

    style.IndentSpacing = Scaled(theme.metrics.indentSpacing, scale);
    style.ColumnsMinSpacing = 6.0f * scale;
    style.LogSliderDeadzone = 4.0f * scale;
    style.ImageBorderSize = 0.0f;
    style.MouseCursorScale = 1.0f;

    style.AntiAliasedLines = true;
    style.AntiAliasedLinesUseTex = true;
    style.AntiAliasedFill = true;
    style.CurveTessellationTol = 1.25f;
    style.CircleTessellationMaxError = 0.30f;

    style.HoverStationaryDelay = 0.15f;
    style.HoverDelayShort = 0.15f;
    style.HoverDelayNormal = 0.40f;
    style.HoverFlagsForTooltipMouse = ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled;
    style.HoverFlagsForTooltipNav = ImGuiHoveredFlags_NoSharedDelay | ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled;

    // ---------------------------------------------------------------------
    // Colors: every ImGuiCol entry is mapped from a semantic token.
    // ---------------------------------------------------------------------
    ImGuiStyle& s = style;
    s.Colors[ImGuiCol_Text] = col(ColorToken::TextPrimary);
    s.Colors[ImGuiCol_TextDisabled] = col(ColorToken::TextDisabled);

    s.Colors[ImGuiCol_WindowBg] = col(ColorToken::Background);
    s.Colors[ImGuiCol_ChildBg] = col(ColorToken::PanelBackground);
    s.Colors[ImGuiCol_PopupBg] = col(ColorToken::SecondaryBackground);
    s.Colors[ImGuiCol_Border] = col(ColorToken::Border);
    s.Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    s.Colors[ImGuiCol_FrameBg] = col(ColorToken::FrameBackground);
    s.Colors[ImGuiCol_FrameBgHovered] = col(ColorToken::FrameHovered);
    s.Colors[ImGuiCol_FrameBgActive] = col(ColorToken::FrameActive);

    s.Colors[ImGuiCol_TitleBg] = col(ColorToken::HeaderBackground);
    s.Colors[ImGuiCol_TitleBgActive] = col(ColorToken::HeaderBackground);
    s.Colors[ImGuiCol_TitleBgCollapsed] = WithAlpha(col(ColorToken::Background), 0.85f);
    s.Colors[ImGuiCol_MenuBarBg] = col(ColorToken::PanelBackground);

    s.Colors[ImGuiCol_ScrollbarBg] = WithAlpha(col(ColorToken::Background), 0.35f);
    s.Colors[ImGuiCol_ScrollbarGrab] = col(ColorToken::BorderStrong);
    s.Colors[ImGuiCol_ScrollbarGrabHovered] = col(ColorToken::TextMuted);
    s.Colors[ImGuiCol_ScrollbarGrabActive] = col(ColorToken::Accent);

    s.Colors[ImGuiCol_CheckMark] = col(ColorToken::Accent);
    s.Colors[ImGuiCol_SliderGrab] = col(ColorToken::Accent);
    s.Colors[ImGuiCol_SliderGrabActive] = col(ColorToken::AccentHovered);

    s.Colors[ImGuiCol_Button] = col(ColorToken::SecondaryBackground);
    s.Colors[ImGuiCol_ButtonHovered] = col(ColorToken::FrameHovered);
    s.Colors[ImGuiCol_ButtonActive] = col(ColorToken::FrameActive);

    s.Colors[ImGuiCol_Header] = WithAlpha(col(ColorToken::Accent), 0.22f);
    s.Colors[ImGuiCol_HeaderHovered] = WithAlpha(col(ColorToken::Accent), 0.32f);
    s.Colors[ImGuiCol_HeaderActive] = WithAlpha(col(ColorToken::Accent), 0.42f);

    s.Colors[ImGuiCol_Separator] = col(ColorToken::Border);
    s.Colors[ImGuiCol_SeparatorHovered] = WithAlpha(col(ColorToken::Accent), 0.50f);
    s.Colors[ImGuiCol_SeparatorActive] = col(ColorToken::Accent);

    s.Colors[ImGuiCol_ResizeGrip] = WithAlpha(col(ColorToken::BorderStrong), 0.60f);
    s.Colors[ImGuiCol_ResizeGripHovered] = WithAlpha(col(ColorToken::Accent), 0.55f);
    s.Colors[ImGuiCol_ResizeGripActive] = col(ColorToken::Accent);

    s.Colors[ImGuiCol_InputTextCursor] = col(ColorToken::AccentHovered);

    s.Colors[ImGuiCol_TabHovered] = col(ColorToken::FrameHovered);
    s.Colors[ImGuiCol_Tab] = col(ColorToken::HeaderBackground);
    s.Colors[ImGuiCol_TabSelected] = col(ColorToken::PanelBackground);
    s.Colors[ImGuiCol_TabSelectedOverline] = col(ColorToken::Accent);
    s.Colors[ImGuiCol_TabDimmed] = col(ColorToken::Background);
    s.Colors[ImGuiCol_TabDimmedSelected] = col(ColorToken::SecondaryBackground);
    s.Colors[ImGuiCol_TabDimmedSelectedOverline] = col(ColorToken::BorderStrong);

    s.Colors[ImGuiCol_DockingPreview] = WithAlpha(col(ColorToken::Accent), 0.35f);
    s.Colors[ImGuiCol_DockingEmptyBg] = col(ColorToken::Background);

    s.Colors[ImGuiCol_PlotLines] = col(ColorToken::Info);
    s.Colors[ImGuiCol_PlotLinesHovered] = col(ColorToken::AccentHovered);
    s.Colors[ImGuiCol_PlotHistogram] = col(ColorToken::Accent);
    s.Colors[ImGuiCol_PlotHistogramHovered] = col(ColorToken::AccentHovered);

    s.Colors[ImGuiCol_TableHeaderBg] = col(ColorToken::HeaderBackground);
    s.Colors[ImGuiCol_TableBorderStrong] = col(ColorToken::BorderStrong);
    s.Colors[ImGuiCol_TableBorderLight] = col(ColorToken::Border);
    s.Colors[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_TableRowBgAlt] = WithAlpha(col(ColorToken::SecondaryBackground), 0.35f);

    s.Colors[ImGuiCol_TextLink] = col(ColorToken::Accent);
    s.Colors[ImGuiCol_TextSelectedBg] = WithAlpha(col(ColorToken::Accent), 0.30f);
    s.Colors[ImGuiCol_TreeLines] = col(ColorToken::BorderStrong);

    s.Colors[ImGuiCol_DragDropTarget] = col(ColorToken::Accent);
    s.Colors[ImGuiCol_DragDropTargetBg] = WithAlpha(col(ColorToken::Accent), 0.12f);
    s.Colors[ImGuiCol_UnsavedMarker] = col(ColorToken::Warning);

    s.Colors[ImGuiCol_NavCursor] = WithAlpha(col(ColorToken::Accent), 0.65f);
    s.Colors[ImGuiCol_NavWindowingHighlight] = WithAlpha(col(ColorToken::TextPrimary), 0.70f);
    s.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.45f);
    s.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.55f);

#if defined(IMGUI_HAS_DOCK)
    // Newer docking-branch additions. Guarded so the applier still compiles
    // against older compatible ImGui versions.
#endif
}

} // namespace AmberUI
