// Amber UI - Basic widgets
// SPDX-License-Identifier: MIT
#include "AmberUI/Widgets.h"

#include "AmberUI/Animation.h"
#include "AmberUI/ThemeManager.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace AmberUI
{
namespace
{
const Theme* ThemeForWidgets()
{
    return ThemeManager::Get().ActiveTheme();
}

float Scale()
{
    const Theme* theme = ThemeForWidgets();
    return theme ? theme->MetricScale() : 1.0f;
}

ImU32 U32(ColorToken token, float alpha = 1.0f)
{
    ImVec4 color = TokenColor(token);
    color.w *= alpha;
    return ImGui::GetColorU32(color);
}

float Luminance(const ImVec4& color)
{
    return 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
}

ImVec4 ContrastText(const ImVec4& background)
{
    const Theme* theme = ThemeForWidgets();
    const ImVec4 light = theme ? theme->Color(ColorToken::TextPrimary) : ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
    const ImVec4 dark = theme ? theme->Color(ColorToken::Background) : ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    return Luminance(background) > 0.55f ? dark : light;
}

void DrawRoundedRect(ImDrawList* draw, const ImVec2& min, const ImVec2& max, float rounding, ImU32 fill,
                     ImU32 border, float border_size)
{
    draw->AddRectFilled(min, max, fill, rounding);
    if (border_size > 0.0f)
        draw->AddRect(min, max, border, rounding, 0, border_size);
}

} // namespace

ImVec4 TokenColor(ColorToken token)
{
    const Theme* theme = ThemeForWidgets();
    if (theme)
        return theme->Color(token);
    // Fallback: keep the library usable before Initialize().
    switch (token)
    {
    case ColorToken::Background: return ImVec4(0.09f, 0.09f, 0.09f, 1.0f);
    case ColorToken::PanelBackground: return ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
    case ColorToken::Accent: return ImVec4(0.77f, 0.64f, 0.42f, 1.0f);
    case ColorToken::TextPrimary: return ImVec4(0.84f, 0.84f, 0.84f, 1.0f);
    case ColorToken::TextSecondary: return ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
    default: return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    }
}

ImU32 TokenU32(ColorToken token, float alpha_multiplier)
{
    ImVec4 color = TokenColor(token);
    color.w *= alpha_multiplier;
    return ImGui::ColorConvertFloat4ToU32(color);
}

float MetricScale()
{
    return Scale();
}

float Metric(float value)
{
    return value * Scale();
}

void SetNextItemWidthFraction(float fraction)
{
    const float available = ImGui::GetContentRegionAvail().x;
    ImGui::SetNextItemWidth(std::max(1.0f, available * std::clamp(fraction, 0.05f, 1.0f)));
}

void TextSecondary(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, TokenColor(ColorToken::TextSecondary));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

void TextDisabled(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, TokenColor(ColorToken::TextDisabled));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

void TextWrappedMuted(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, TokenColor(ColorToken::TextSecondary));
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}

// =============================================================================
// Buttons
// =============================================================================
bool ButtonEx(const char* label, ButtonVariant variant, const ImVec2& size)
{
    const Theme* theme = ThemeForWidgets();
    if (!theme)
        return ImGui::Button(label, size);

    const ImGuiStyle& style = ImGui::GetStyle();
    const float scale = theme->MetricScale();
    const ImVec2 text_size = ImGui::CalcTextSize(label, nullptr, true);
    ImVec2 button_size = size;
    if (button_size.x <= 0.0f)
        button_size.x = text_size.x + style.FramePadding.x * 2.0f;
    if (button_size.y <= 0.0f)
        button_size.y = std::max(theme->metrics.controlHeight * scale, text_size.y + style.FramePadding.y * 2.0f);

    ImVec4 base, hovered, active, text, border;
    switch (variant)
    {
    case ButtonVariant::Primary:
        base = theme->Color(ColorToken::Accent);
        hovered = theme->Color(ColorToken::AccentHovered);
        active = theme->Color(ColorToken::AccentActive);
        text = ContrastText(base);
        border = WithAlpha(theme->Color(ColorToken::AccentHovered), 0.0f);
        break;
    case ButtonVariant::Ghost:
        base = WithAlpha(theme->Color(ColorToken::PanelBackground), 0.0f);
        hovered = WithAlpha(theme->Color(ColorToken::Accent), 0.14f);
        active = WithAlpha(theme->Color(ColorToken::Accent), 0.24f);
        text = theme->Color(ColorToken::TextSecondary);
        border = WithAlpha(theme->Color(ColorToken::Border), 0.0f);
        break;
    case ButtonVariant::Danger:
        base = theme->Color(ColorToken::Error);
        hovered = Lighten(base, 0.07f);
        active = Darken(base, 0.06f);
        text = ContrastText(base);
        border = WithAlpha(base, 0.0f);
        break;
    case ButtonVariant::Secondary:
    default:
        base = theme->Color(ColorToken::SecondaryBackground);
        hovered = theme->Color(ColorToken::FrameHovered);
        active = theme->Color(ColorToken::FrameActive);
        text = theme->Color(ColorToken::TextPrimary);
        border = theme->Color(ColorToken::Border);
        break;
    }

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const bool hovered_now = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + button_size.x, pos.y + button_size.y)) &&
                             ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
    const float hover_t = Animation::Hover(ImGui::GetID(label), hovered_now);
    ImVec4 animated = LerpColor(base, hovered, hover_t);
    if (variant == ButtonVariant::Ghost)
        animated = LerpColor(base, hovered, hover_t);

    const bool has_border = variant == ButtonVariant::Secondary;
    ImGui::PushStyleColor(ImGuiCol_Button, animated);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
    ImGui::PushStyleColor(ImGuiCol_Text, text);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.FrameRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, has_border ? theme->metrics.windowBorderSize * scale : 0.0f);
    if (has_border)
        ImGui::PushStyleColor(ImGuiCol_Border, border);
    else
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

    const bool clicked = ImGui::Button(label, button_size);

    ImGui::PopStyleColor(has_border ? 5 : 5);
    ImGui::PopStyleVar(2);
    return clicked;
}

bool Button(const char* label, const ImVec2& size)
{
    return ButtonEx(label, ButtonVariant::Secondary, size);
}

bool PrimaryButton(const char* label, const ImVec2& size)
{
    return ButtonEx(label, ButtonVariant::Primary, size);
}

bool SecondaryButton(const char* label, const ImVec2& size)
{
    return ButtonEx(label, ButtonVariant::Secondary, size);
}

bool GhostButton(const char* label, const ImVec2& size)
{
    return ButtonEx(label, ButtonVariant::Ghost, size);
}

bool IconButton(Icon icon, const char* tooltip, const ImVec2& size)
{
    const Theme* theme = ThemeForWidgets();
    const float scale = Scale();
    const float button_height = size.y > 0.0f ? size.y : (theme ? theme->metrics.controlHeight * scale : 24.0f);
    const float button_width = size.x > 0.0f ? size.x : button_height;

    ImGui::PushID(static_cast<int>(icon));
    ImGui::PushID(tooltip ? tooltip : "icon");
    const bool clicked = ImGui::InvisibleButton("##icon_button", ImVec2(button_width, button_height));
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    const ImVec4 base = theme ? WithAlpha(theme->Color(ColorToken::PanelBackground), 0.0f) : ImVec4(0, 0, 0, 0);
    const ImVec4 hover = theme ? WithAlpha(theme->Color(ColorToken::Accent), 0.14f) : ImVec4(0.5f, 0.5f, 0.5f, 0.2f);
    const ImVec4 pressed = theme ? WithAlpha(theme->Color(ColorToken::Accent), 0.24f) : ImVec4(0.5f, 0.5f, 0.5f, 0.3f);
    const float hover_t = Animation::Hover(ImGui::GetID("##icon_hover"), hovered);
    ImVec4 background = LerpColor(base, held ? pressed : hover, hover_t);
    if (theme)
        DrawRoundedRect(draw, min, max, ImGui::GetStyle().FrameRounding, ImGui::GetColorU32(background), 0, 0.0f);

    const ImVec4 icon_color = held ? theme->Color(ColorToken::AccentHovered)
                                   : (hovered ? theme->Color(ColorToken::TextPrimary) : theme->Color(ColorToken::TextSecondary));
    DrawIcon(draw, icon, ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f), button_height * 0.48f,
             ImGui::GetColorU32(icon_color), 1.5f);

    if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        ImGui::SetTooltip("%s", tooltip);
    ImGui::PopID();
    ImGui::PopID();
    return clicked;
}

// =============================================================================
// Toggles / inputs
// =============================================================================
bool ToggleSwitch(const char* label, bool* value, float scale)
{
    if (!value)
        return false;
    const Theme* theme = ThemeForWidgets();
    ImGui::PushID(label);
    const ImGuiStyle& style = ImGui::GetStyle();
    const float s = Scale() * (scale > 0.0f ? scale : 1.0f);
    const float height = theme ? theme->metrics.rowHeight * s : 22.0f * s;
    const float switch_width = 34.0f * s;
    const float switch_height = 16.0f * s;
    const ImVec2 text_size = ImGui::CalcTextSize(label);
    const ImVec2 row_size(text_size.x + style.ItemInnerSpacing.x + switch_width, height);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const bool clicked = ImGui::InvisibleButton("##toggle", row_size);
    const bool hovered = ImGui::IsItemHovered();
    if (clicked)
        *value = !*value;

    const float t = Animation::Float(ImGui::GetID("##toggle_anim"), *value ? 1.0f : 0.0f, 18.0f);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 switch_min(pos.x + row_size.x - switch_width, pos.y + (height - switch_height) * 0.5f);
    const ImVec2 switch_max(switch_min.x + switch_width, switch_min.y + switch_height);
    const float rounding = switch_height * 0.5f;

    const ImVec4 off_bg = theme ? theme->Color(ColorToken::FrameBackground) : ImVec4(0.2f, 0.2f, 0.2f, 1);
    const ImVec4 on_bg = theme ? theme->Color(ColorToken::Accent) : ImVec4(0.8f, 0.65f, 0.4f, 1);
    const ImVec4 track = LerpColor(off_bg, on_bg, t);
    draw->AddRectFilled(switch_min, switch_max, ImGui::GetColorU32(track), rounding);
    if (theme)
        draw->AddRect(switch_min, switch_max, U32(hovered ? ColorToken::BorderStrong : ColorToken::Border), rounding, 0, 1.0f);

    const float knob_radius = switch_height * 0.5f - 2.0f * s;
    const float knob_x = switch_min.x + switch_height * 0.5f + t * (switch_width - switch_height);
    const ImVec2 knob_center(knob_x, (switch_min.y + switch_max.y) * 0.5f);
    const ImVec4 knob_color = t > 0.5f ? ContrastText(track) : (theme ? theme->Color(ColorToken::TextMuted) : ImVec4(0.7f, 0.7f, 0.7f, 1));
    draw->AddCircleFilled(knob_center, std::max(1.0f, knob_radius), ImGui::GetColorU32(knob_color), 24);

    const ImVec4 label_color = theme ? theme->Color(hovered ? ColorToken::TextPrimary : ColorToken::TextSecondary)
                                     : ImVec4(0.8f, 0.8f, 0.8f, 1);
    draw->AddText(ImVec2(pos.x, pos.y + (height - text_size.y) * 0.5f), ImGui::GetColorU32(label_color), label);
    ImGui::PopID();
    return clicked;
}

bool Checkbox(const char* label, bool* value)
{
    return ImGui::Checkbox(label, value);
}

bool RadioButton(const char* label, int* value, int option_value)
{
    if (!value)
        return false;
    if (ImGui::RadioButton(label, *value == option_value))
    {
        *value = option_value;
        return true;
    }
    return false;
}

bool SearchBar(const char* id, char* buffer, size_t buffer_size, const char* hint)
{
    const Theme* theme = ThemeForWidgets();
    const float s = Scale();
    const ImGuiStyle& style = ImGui::GetStyle();
    const float icon_space = (theme ? theme->metrics.rowHeight : 20.0f) * s;
    ImGui::PushID(id);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x + icon_space, style.FramePadding.y));
    ImGui::SetNextItemWidth(-FLT_MIN);
    const bool changed = ImGui::InputTextWithHint("##search", hint ? hint : "Search...", buffer, buffer_size);
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    DrawIcon(draw, Icon::Search, ImVec2(min.x + icon_space * 0.55f, (min.y + max.y) * 0.5f), icon_space * 0.5f,
             U32(ColorToken::TextMuted), 1.4f);

    const ImVec2 cursor_after = ImGui::GetCursorScreenPos();
    if (buffer && buffer[0] != '\0')
    {
        const float clear_size = (max.y - min.y) - 4.0f * s;
        const ImVec2 clear_min(max.x - clear_size - 4.0f * s, min.y + 2.0f * s);
        ImGui::SetCursorScreenPos(clear_min);
        const bool cleared = ImGui::InvisibleButton("##clear", ImVec2(clear_size, clear_size));
        const ImVec2 center(clear_min.x + clear_size * 0.5f, clear_min.y + clear_size * 0.5f);
        DrawIcon(draw, Icon::Close, center, clear_size * 0.5f,
                 U32(ImGui::IsItemHovered() ? ColorToken::TextPrimary : ColorToken::TextMuted), 1.4f);
        if (cleared)
        {
            buffer[0] = '\0';
            ImGui::SetKeyboardFocusHere(-1);
        }
        ImGui::SetCursorScreenPos(cursor_after);
    }
    ImGui::PopStyleVar();
    ImGui::PopID();
    return changed;
}

bool TextInput(const char* label, char* buffer, size_t buffer_size, const char* hint, ImGuiInputTextFlags flags)
{
    if (!buffer)
        return false;
    if (hint)
        return ImGui::InputTextWithHint(label, hint, buffer, buffer_size, flags);
    return ImGui::InputText(label, buffer, buffer_size, flags);
}

bool PasswordInput(const char* label, char* buffer, size_t buffer_size)
{
    return ImGui::InputText(label, buffer, buffer_size, ImGuiInputTextFlags_Password);
}

bool NumberInput(const char* label, float* value, float step, float min_value, float max_value, const char* format)
{
    if (!value)
        return false;
    ImGui::SetNextItemWidth(ImGui::CalcItemWidth());
    ImGui::PushID(label);
    float speed = step > 0.0f ? step : 1.0f;
    bool changed = ImGui::InputFloat("##number", value, speed, speed * 10.0f, format);
    if (min_value != max_value)
    {
        const float clamped = std::clamp(*value, std::min(min_value, max_value), std::max(min_value, max_value));
        if (clamped != *value)
        {
            *value = clamped;
            changed = true;
        }
    }
    ImGui::PopID();
    if (label && label[0])
    {
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::TextUnformatted(label);
    }
    return changed;
}

bool FloatSlider(const char* label, float* value, float min_value, float max_value, const char* format)
{
    if (!value)
        return false;
    ImGui::SetNextItemWidth(ImGui::CalcItemWidth());
    return ImGui::SliderFloat(label, value, min_value, max_value, format);
}

bool RangeSlider(const char* label, float* min_value, float* max_value, float min_limit, float max_limit,
                 const char* format)
{
    if (!min_value || !max_value)
        return false;
    const Theme* theme = ThemeForWidgets();
    const float s = Scale();
    const ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushID(label);

    const float available = ImGui::CalcItemWidth();
    const float text_height = ImGui::GetTextLineHeight();
    const float height = std::max((theme ? theme->metrics.controlHeight : 24.0f) * s, text_height + style.FramePadding.y * 2.0f);
    ImGui::InvisibleButton("##range", ImVec2(available, height));
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    const float range = max_limit - min_limit;
    const float safe_range = std::fabs(range) < 1e-6f ? 1.0f : range;
    const float padding = 6.0f * s;
    const float track_left = min.x + padding;
    const float track_right = max.x - padding;
    const float track_y = (min.y + max.y) * 0.5f;
    const float track_height = 4.0f * s;

    draw->AddRectFilled(ImVec2(track_left, track_y - track_height * 0.5f),
                        ImVec2(track_right, track_y + track_height * 0.5f),
                        U32(ColorToken::FrameBackground), track_height * 0.5f);
    const float t_min = std::clamp((*min_value - min_limit) / safe_range, 0.0f, 1.0f);
    const float t_max = std::clamp((*max_value - min_limit) / safe_range, 0.0f, 1.0f);
    const float x_min = track_left + t_min * (track_right - track_left);
    const float x_max = track_left + t_max * (track_right - track_left);
    draw->AddRectFilled(ImVec2(x_min, track_y - track_height * 0.5f), ImVec2(x_max, track_y + track_height * 0.5f),
                        U32(ColorToken::Accent, 0.75f), track_height * 0.5f);

    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();
    ImGuiID drag_id = ImGui::GetID("##range_drag");
    int& dragging = *ImGui::GetStateStorage()->GetIntRef(drag_id, -1);
    if (ImGui::IsItemActivated() && hovered)
    {
        const float mouse_x = ImGui::GetIO().MousePos.x;
        dragging = std::fabs(mouse_x - x_min) <= std::fabs(mouse_x - x_max) ? 0 : 1;
    }
    if (active && dragging >= 0)
    {
        const float mouse_x = ImGui::GetIO().MousePos.x;
        float t = (mouse_x - track_left) / (track_right - track_left);
        t = std::clamp(t, 0.0f, 1.0f);
        const float new_value = min_limit + t * range;
        if (dragging == 0)
            *min_value = std::min(new_value, *max_value);
        else
            *max_value = std::max(new_value, *min_value);
    }
    if (!active)
        dragging = -1;

    const ImU32 handle_color = U32(active || hovered ? ColorToken::AccentHovered : ColorToken::TextPrimary);
    const float r = 5.0f * s;
    draw->AddCircleFilled(ImVec2(x_min, track_y), r, handle_color);
    draw->AddCircleFilled(ImVec2(x_max, track_y), r, handle_color);
    draw->AddCircle(ImVec2(x_min, track_y), r, U32(ColorToken::Background), 0, 1.0f * s);
    draw->AddCircle(ImVec2(x_max, track_y), r, U32(ColorToken::Background), 0, 1.0f * s);

    char min_text[32];
    char max_text[32];
    std::snprintf(min_text, sizeof(min_text), format ? format : "%.2f", *min_value);
    std::snprintf(max_text, sizeof(max_text), format ? format : "%.2f", *max_value);
    if (active || hovered)
    {
        draw->AddText(ImVec2(x_min - ImGui::CalcTextSize(min_text).x * 0.5f, min.y - ImGui::GetTextLineHeight() - 2.0f * s),
                      U32(ColorToken::TextSecondary), min_text);
        draw->AddText(ImVec2(x_max - ImGui::CalcTextSize(max_text).x * 0.5f, min.y - ImGui::GetTextLineHeight() - 2.0f * s),
                      U32(ColorToken::TextSecondary), max_text);
    }

    const bool changed = ImGui::IsItemActive() || ImGui::IsItemActivated();
    ImGui::PopID();
    if (label && label[0])
    {
        ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
        ImGui::TextUnformatted(label);
    }
    return changed;
}

bool Dropdown(const char* label, int* current, const char* const items[], int item_count)
{
    if (!current || !items)
        return false;
    bool changed = false;
    const char* preview = (*current >= 0 && *current < item_count) ? items[*current] : "";
    ImGui::SetNextItemWidth(ImGui::CalcItemWidth());
    if (ImGui::BeginCombo(label, preview))
    {
        for (int i = 0; i < item_count; ++i)
        {
            const bool selected = (*current == i);
            if (ImGui::Selectable(items[i], selected))
            {
                *current = i;
                changed = true;
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool Dropdown(const char* label, int* current, const std::vector<std::string>& items)
{
    if (!current)
        return false;
    bool changed = false;
    const char* preview = (*current >= 0 && *current < static_cast<int>(items.size())) ? items[static_cast<size_t>(*current)].c_str() : "";
    ImGui::SetNextItemWidth(ImGui::CalcItemWidth());
    if (ImGui::BeginCombo(label, preview))
    {
        for (int i = 0; i < static_cast<int>(items.size()); ++i)
        {
            const bool selected = (*current == i);
            if (ImGui::Selectable(items[static_cast<size_t>(i)].c_str(), selected))
            {
                *current = i;
                changed = true;
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

// =============================================================================
// Display components
// =============================================================================
void Badge(const char* text, BadgeStyle badge_style)
{
    const Theme* theme = ThemeForWidgets();
    ColorToken token = ColorToken::TextSecondary;
    switch (badge_style)
    {
    case BadgeStyle::Accent: token = ColorToken::Accent; break;
    case BadgeStyle::Success: token = ColorToken::Success; break;
    case BadgeStyle::Warning: token = ColorToken::Warning; break;
    case BadgeStyle::Error: token = ColorToken::Error; break;
    case BadgeStyle::Info: token = ColorToken::Info; break;
    case BadgeStyle::Neutral:
    default: token = ColorToken::TextMuted; break;
    }
    const ImVec4 color = theme ? theme->Color(token) : ImVec4(0.7f, 0.7f, 0.7f, 1);
    const float s = Scale();
    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 text_size = ImGui::CalcTextSize(text);
    const ImVec2 padding(6.0f * s, 2.0f * s);
    const ImVec2 size(text_size.x + padding.x * 2.0f, text_size.y + padding.y * 2.0f);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const float rounding = std::min(style.FrameRounding, size.y * 0.5f);
    draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImGui::GetColorU32(WithAlpha(color, 0.16f)), rounding);
    draw->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImGui::GetColorU32(WithAlpha(color, 0.45f)), rounding, 0, 1.0f);
    draw->AddText(ImVec2(pos.x + padding.x, pos.y + padding.y), ImGui::GetColorU32(color), text);
    ImGui::Dummy(size);
}

void Tooltip(const char* text)
{
    if (!ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        return;
    const Theme* theme = ThemeForWidgets();
    ImGui::PushStyleColor(ImGuiCol_PopupBg, theme ? theme->Color(ColorToken::SecondaryBackground) : ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
    ImGui::PushStyleColor(ImGuiCol_Border, theme ? theme->Color(ColorToken::BorderStrong) : ImGui::GetStyleColorVec4(ImGuiCol_Border));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * Scale(), 6.0f * Scale()));
    ImGui::BeginTooltip();
    ImGui::PushStyleColor(ImGuiCol_Text, theme ? theme->Color(ColorToken::TextPrimary) : ImGui::GetStyleColorVec4(ImGuiCol_Text));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
    ImGui::EndTooltip();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void LoadingSpinner(const char* id, float radius, float thickness)
{
    ImGui::PushID(id);
    const float s = Scale();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float size = radius * 2.0f + thickness * 2.0f;
    ImGui::Dummy(ImVec2(size * s, size * s));
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 center(pos.x + size * s * 0.5f, pos.y + size * s * 0.5f);
    const float fraction = static_cast<float>(std::fmod(ImGui::GetTime() * 1.6, 1.0));
    const float start = static_cast<float>(ImGui::GetTime()) * 3.0f;
    draw->PathClear();
    draw->PathArcTo(center, radius * s, start, start + 1.6f + fraction, 32);
    draw->PathStroke(U32(ColorToken::Accent), 0, thickness * s);
    ImGui::PopID();
    ImGui::SameLine(0.0f, 6.0f * s);
}

void ProgressBar(float fraction, const ImVec2& size, const char* overlay, bool animated)
{
    const Theme* theme = ThemeForWidgets();
    const float s = Scale();
    const float height = std::max(8.0f * s, size.y > 0.0f ? size.y : (theme ? theme->metrics.rowHeight : 22.0f) * s * 0.7f);
    float width = size.x;
    if (width <= 0.0f)
        width = ImGui::GetContentRegionAvail().x;
    else if (width < 0.0f)
        width = ImGui::GetContentRegionAvail().x + width;

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float target = std::clamp(fraction, 0.0f, 1.0f);
    const float value = animated ? Animation::Float(ImGui::GetID(overlay ? overlay : "##progress"), target, 10.0f) : target;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 max(pos.x + width, pos.y + height);
    draw->AddRectFilled(pos, max, U32(ColorToken::FrameBackground), height * 0.5f);
    if (value > 0.0f)
    {
        ImVec2 fill_max(pos.x + width * value, max.y);
        fill_max.x = std::max(fill_max.x, pos.x + height);
        draw->AddRectFilled(pos, fill_max, U32(ColorToken::Accent), height * 0.5f);
    }
    if (overlay && overlay[0])
    {
        const ImVec2 text_size = ImGui::CalcTextSize(overlay);
        const bool over_fill = value > 0.5f;
        draw->AddText(ImVec2(pos.x + (width - text_size.x) * 0.5f, pos.y + (height - text_size.y) * 0.5f),
                      over_fill ? U32(ColorToken::Background) : U32(ColorToken::TextSecondary), overlay);
    }
    ImGui::Dummy(ImVec2(width, height));
}

bool TabBar(const char* id, const char* const labels[], int count, int* active, bool full_width)
{
    if (!labels || count <= 0 || !active)
        return false;
    const Theme* theme = ThemeForWidgets();
    const float s = Scale();
    const ImGuiStyle& style = ImGui::GetStyle();
    const float height = (theme ? theme->metrics.rowHeight : 22.0f) * s;
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const float available = ImGui::GetContentRegionAvail().x;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddLine(ImVec2(start.x, start.y + height), ImVec2(start.x + available, start.y + height),
                  U32(ColorToken::Border), 1.0f);

    float x = start.x;
    bool changed = false;
    ImGui::PushID(id);
    for (int i = 0; i < count; ++i)
    {
        const ImVec2 text_size = ImGui::CalcTextSize(labels[i]);
        const float width = full_width ? available / static_cast<float>(count)
                                       : std::max(text_size.x + style.FramePadding.x * 3.0f, 48.0f * s);
        ImGui::PushID(i);
        ImGui::SetCursorScreenPos(ImVec2(x, start.y));
        const bool clicked = ImGui::InvisibleButton("##tab", ImVec2(width, height));
        const bool hovered = ImGui::IsItemHovered();
        if (clicked && *active != i)
        {
            *active = i;
            changed = true;
        }
        const bool selected = *active == i;
        if (hovered && !selected)
            draw->AddRectFilled(ImVec2(x, start.y), ImVec2(x + width, start.y + height),
                                U32(ColorToken::HeaderBackground, 0.6f));
        if (selected)
        {
            draw->AddRectFilled(ImVec2(x, start.y), ImVec2(x + width, start.y + height), U32(ColorToken::PanelBackground));
            draw->AddLine(ImVec2(x + 6.0f * s, start.y + height), ImVec2(x + width - 6.0f * s, start.y + height),
                          U32(ColorToken::Accent), 2.0f * s);
        }
        const ImVec4 text_color = theme ? theme->Color(selected ? ColorToken::TextPrimary
                                                               : (hovered ? ColorToken::TextPrimary : ColorToken::TextSecondary))
                                        : ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
        draw->AddText(ImVec2(x + (width - text_size.x) * 0.5f, start.y + (height - text_size.y) * 0.5f),
                      ImGui::GetColorU32(text_color), labels[i]);
        ImGui::PopID();
        x += width;
    }
    ImGui::PopID();
    ImGui::SetCursorScreenPos(ImVec2(start.x, start.y + height + style.ItemSpacing.y));
    return changed;
}

bool Breadcrumb(const char* id, const char* const labels[], int count, int* selected)
{
    if (!labels || count <= 0)
        return false;
    ImGui::PushID(id);
    bool changed = false;
    for (int i = 0; i < count; ++i)
    {
        if (i > 0)
        {
            ImGui::SameLine(0.0f, 4.0f * Scale());
            ImGui::PushStyleColor(ImGuiCol_Text, TokenColor(ColorToken::TextDisabled));
            ImGui::TextUnformatted("/");
            ImGui::PopStyleColor();
            ImGui::SameLine(0.0f, 4.0f * Scale());
        }
        const bool is_last = i == count - 1;
        ImGui::PushStyleColor(ImGuiCol_Text, TokenColor(is_last ? ColorToken::TextPrimary : ColorToken::TextSecondary));
        if (ImGui::Selectable(labels[i], is_last, ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (selected)
                *selected = i;
            changed = true;
        }
        ImGui::PopStyleColor();
    }
    ImGui::PopID();
    return changed;
}

void Divider(const char* label)
{
    const Theme* theme = ThemeForWidgets();
    const float s = Scale();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    const float height = ImGui::GetTextLineHeightWithSpacing();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const float y = pos.y + height * 0.5f;
    if (!label || !label[0])
    {
        draw->AddLine(ImVec2(pos.x, y), ImVec2(pos.x + width, y), U32(ColorToken::Border), 1.0f);
        ImGui::Dummy(ImVec2(width, height * 0.5f));
        return;
    }
    const ImVec2 text_size = ImGui::CalcTextSize(label);
    const float center = pos.x + width * 0.5f;
    draw->AddLine(ImVec2(pos.x, y), ImVec2(center - text_size.x * 0.5f - 8.0f * s, y), U32(ColorToken::Border));
    draw->AddLine(ImVec2(center + text_size.x * 0.5f + 8.0f * s, y), ImVec2(pos.x + width, y), U32(ColorToken::Border));
    draw->AddText(ImVec2(center - text_size.x * 0.5f, pos.y + (height - text_size.y) * 0.5f),
                  ImGui::GetColorU32(theme ? theme->Color(ColorToken::TextMuted) : ImVec4(0.6f, 0.6f, 0.6f, 1)), label);
    ImGui::Dummy(ImVec2(width, height));
}

void SectionHeader(const char* label, const char* right_text)
{
    const Theme* theme = ThemeForWidgets();
    const float s = Scale();
    ImGui::PushStyleColor(ImGuiCol_Text, theme ? theme->Color(ColorToken::TextPrimary) : ImVec4(0.9f, 0.9f, 0.9f, 1));
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    if (right_text && right_text[0])
    {
        const float right_width = ImGui::CalcTextSize(right_text).x;
        ImGui::SameLine(ImGui::GetContentRegionMax().x - right_width);
        ImGui::PushStyleColor(ImGuiCol_Text, theme ? theme->Color(ColorToken::TextMuted) : ImVec4(0.6f, 0.6f, 0.6f, 1));
        ImGui::TextUnformatted(right_text);
        ImGui::PopStyleColor();
    }
    ImGui::PushStyleColor(ImGuiCol_Separator, theme ? theme->Color(ColorToken::Border) : ImVec4(0.3f, 0.3f, 0.3f, 1));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0.0f, 2.0f * s));
}

} // namespace AmberUI
