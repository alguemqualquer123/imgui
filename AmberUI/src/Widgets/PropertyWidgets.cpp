// Amber UI - Property grid, vector and curve widgets
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

float S()
{
    const Theme* theme = ThemeForWidgets();
    return theme ? theme->MetricScale() : 1.0f;
}

ImU32 U32(ColorToken token, float alpha = 1.0f)
{
    ImVec4 color = ThemeForWidgets() ? ThemeForWidgets()->Color(token) : ImVec4(0.7f, 0.7f, 0.7f, 1);
    color.w *= alpha;
    return ImGui::GetColorU32(color);
}

ImVec4 TokenColorSafe(ColorToken token)
{
    const Theme* theme = ThemeForWidgets();
    return theme ? theme->Color(token) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
}

void CellText(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, TokenColorSafe(ColorToken::TextSecondary));
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

float LabelWidth()
{
    return 96.0f * S();
}

void DrawInlineLabel(const char* label, float width)
{
    if (!label || !label[0])
        return;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(width, ImGui::GetFrameHeight()));
    ImGui::GetWindowDrawList()->AddText(
        ImVec2(pos.x, pos.y + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) * 0.5f),
        U32(ColorToken::TextSecondary), label);
}
} // namespace

// =============================================================================
// Property grid
// =============================================================================
bool BeginPropertyGrid(const char* id, float label_width_ratio, ImGuiTableFlags extra_flags)
{
    const float available = ImGui::GetContentRegionAvail().x;
    const ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings |
                                  ImGuiTableFlags_NoPadOuterX | extra_flags;
    if (!ImGui::BeginTable(id, 2, flags))
        return false;
    ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed,
                            available * std::clamp(label_width_ratio, 0.15f, 0.8f));
    ImGui::TableSetupColumn("##value", ImGuiTableColumnFlags_WidthStretch);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f * S(), 2.0f * S()));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f * S(), 3.0f * S()));
    return true;
}

void EndPropertyGrid()
{
    ImGui::PopStyleVar(2);
    ImGui::EndTable();
}

bool PropertySection(const char* label, bool default_open)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed;
    if (default_open)
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    const Theme* theme = ThemeForWidgets();
    if (theme)
    {
        ImGui::PushStyleColor(ImGuiCol_Header, WithAlpha(theme->Color(ColorToken::HeaderBackground), 0.85f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, theme->Color(ColorToken::FrameHovered));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, theme->Color(ColorToken::FrameActive));
    }
    const bool open = ImGui::CollapsingHeader(label, flags);
    if (theme)
        ImGui::PopStyleColor(3);
    return open;
}

bool PropertyGroup(const char* label)
{
    return ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed);
}

void EndPropertyGroup()
{
    ImGui::TreePop();
}

namespace
{
bool BeginRow(const char* label)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    CellText(label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::PushID(label);
    return true;
}

void EndRow(bool value_is_true)
{
    (void)value_is_true;
    ImGui::PopID();
}
} // namespace

bool PropertyRowFloat(const char* label, float* value, float speed, float min_value, float max_value,
                      const char* format)
{
    if (!value)
        return false;
    BeginRow(label);
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool changed;
    if (min_value != max_value)
        changed = ImGui::SliderFloat("##value", value, min_value, max_value, format);
    else
        changed = ImGui::DragFloat("##value", value, speed, 0.0f, 0.0f, format);
    EndRow(changed);
    return changed;
}

bool PropertyRowInt(const char* label, int* value, int min_value, int max_value)
{
    if (!value)
        return false;
    BeginRow(label);
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool changed;
    if (min_value != max_value)
        changed = ImGui::SliderInt("##value", value, min_value, max_value);
    else
        changed = ImGui::DragInt("##value", value, 0.2f);
    EndRow(changed);
    return changed;
}

bool PropertyRowBool(const char* label, bool* value)
{
    if (!value)
        return false;
    BeginRow(label);
    const bool changed = ImGui::Checkbox("##value", value);
    EndRow(changed);
    return changed;
}

bool PropertyRowText(const char* label, char* buffer, size_t buffer_size)
{
    if (!buffer)
        return false;
    BeginRow(label);
    const bool changed = ImGui::InputText("##value", buffer, buffer_size);
    EndRow(changed);
    return changed;
}

bool PropertyRowColor(const char* label, ImVec4* color, bool with_alpha)
{
    if (!color)
        return false;
    BeginRow(label);
    const ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf |
                                      (with_alpha ? ImGuiColorEditFlags_AlphaBar : 0);
    bool changed = ImGui::ColorEdit4("##value", &color->x, flags);
    EndRow(changed);
    return changed;
}

bool PropertyRowCombo(const char* label, int* current, const char* const items[], int item_count)
{
    if (!current || !items)
        return false;
    BeginRow(label);
    const char* preview = (*current >= 0 && *current < item_count) ? items[*current] : "";
    bool changed = false;
    if (ImGui::BeginCombo("##value", preview))
    {
        for (int i = 0; i < item_count; ++i)
        {
            if (ImGui::Selectable(items[i], *current == i))
            {
                *current = i;
                changed = true;
            }
            if (*current == i)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    EndRow(changed);
    return changed;
}

bool PropertyRowCustom(const char* label, const std::function<bool()>& widget)
{
    BeginRow(label);
    const bool changed = widget ? widget() : false;
    EndRow(changed);
    return changed;
}

// =============================================================================
// Vector controls
// =============================================================================
namespace
{
bool VectorControl(const char* label, float* values, int count, float speed, const char* format)
{
    if (!values)
        return false;
    ImGui::PushID(label);
    const float width = LabelWidth();
    DrawInlineLabel(label, width);
    if (label && label[0])
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

    const ColorToken axis_tokens[4] = { ColorToken::AxisX, ColorToken::AxisY, ColorToken::AxisZ, ColorToken::TextMuted };
    const char* axis_names[4] = { "X", "Y", "Z", "W" };
    const float available = ImGui::GetContentRegionAvail().x;
    const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    const float field_width = std::max(24.0f, (available - spacing * (count - 1)) / static_cast<float>(count));
    const float letter_space = 11.0f * S();

    bool changed = false;
    for (int i = 0; i < count; ++i)
    {
        if (i > 0)
            ImGui::SameLine(0.0f, spacing);
        ImGui::PushID(i);
        const ImVec4 axis_color = TokenColorSafe(axis_tokens[i]);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, WithAlpha(axis_color, 0.10f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, WithAlpha(axis_color, 0.18f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, WithAlpha(axis_color, 0.26f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x + letter_space,
                                                               ImGui::GetStyle().FramePadding.y));
        ImGui::SetNextItemWidth(field_width);
        changed |= ImGui::DragFloat("##component", &values[i], speed, 0.0f, 0.0f, format);
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 text_size = ImGui::CalcTextSize(axis_names[i]);
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(min.x + 5.0f * S(), min.y + (ImGui::GetItemRectSize().y - text_size.y) * 0.5f),
            ImGui::GetColorU32(axis_color), axis_names[i]);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    ImGui::PopID();
    return changed;
}
} // namespace

bool Vector2Control(const char* label, float values[2], float speed, const char* format)
{
    return VectorControl(label, values, 2, speed, format);
}

bool Vector3Control(const char* label, float values[3], float speed, const char* format)
{
    return VectorControl(label, values, 3, speed, format);
}

bool Vector4Control(const char* label, float values[4], float speed, const char* format)
{
    return VectorControl(label, values, 4, speed, format);
}

bool TransformControl(const char* label, float position[3], float rotation[3], float scale[3])
{
    if (!position || !rotation || !scale)
        return false;
    ImGui::PushID(label);
    bool changed = false;
    changed |= Vector3Control("Position", position, 0.05f, "%.2f");
    changed |= Vector3Control("Rotation", rotation, 0.5f, "%.1f");
    changed |= Vector3Control("Scale", scale, 0.01f, "%.2f");
    ImGui::PopID();
    return changed;
}

bool ColorField(const char* label, ImVec4* color, bool with_alpha)
{
    if (!color)
        return false;
    ImGui::PushID(label);
    const float width = LabelWidth();
    DrawInlineLabel(label, width);
    if (label && label[0])
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

    const float height = ImGui::GetFrameHeight();
    const ImVec2 swatch_size(46.0f * S(), height);
    const ImGuiColorEditFlags preview_flags = ImGuiColorEditFlags_AlphaPreviewHalf |
                                              (with_alpha ? 0 : ImGuiColorEditFlags_NoAlpha);
    bool changed = false;
    if (ImGui::ColorButton("##swatch", *color, preview_flags, swatch_size))
        ImGui::OpenPopup("##color_popup");
    ImGui::SameLine(0.0f, 8.0f * S());
    char hex[16];
    std::snprintf(hex, sizeof(hex), "%s", ColorToHex(*color, with_alpha).c_str());
    ImGui::AlignTextToFramePadding();
    ImGui::PushStyleColor(ImGuiCol_Text, TokenColorSafe(ColorToken::TextMuted));
    ImGui::TextUnformatted(hex);
    ImGui::PopStyleColor();

    if (ImGui::BeginPopup("##color_popup"))
    {
        const ImGuiColorEditFlags picker_flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar;
        changed |= ImGui::ColorPicker4("##picker", &color->x, picker_flags | (with_alpha ? 0 : ImGuiColorEditFlags_NoAlpha));
        ImGui::EndPopup();
    }
    ImGui::PopID();
    return changed;
}

bool RangeControl(const char* label, float* min_value, float* max_value, float low_limit, float high_limit)
{
    if (!min_value || !max_value)
        return false;
    ImGui::PushID(label);
    const float width = LabelWidth();
    DrawInlineLabel(label, width);
    if (label && label[0])
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    const float available = ImGui::GetContentRegionAvail().x;
    const float field_width = std::max(30.0f, (available - spacing) * 0.5f);
    ImGui::SetNextItemWidth(field_width);
    bool changed = ImGui::DragFloat("##min", min_value, 0.01f, low_limit, high_limit, "%.3f");
    ImGui::SameLine(0.0f, spacing);
    ImGui::SetNextItemWidth(field_width);
    changed |= ImGui::DragFloat("##max", max_value, 0.01f, low_limit, high_limit, "%.3f");
    if (*min_value > *max_value)
        std::swap(*min_value, *max_value);
    ImGui::PopID();
    return changed;
}

// =============================================================================
// Gradient
// =============================================================================
ImVec4 Gradient::Evaluate(float t) const
{
    if (stops.empty())
        return ImVec4(1, 1, 1, 1);
    std::vector<GradientStop> sorted = stops;
    std::sort(sorted.begin(), sorted.end(), [](const GradientStop& a, const GradientStop& b) {
        return a.position < b.position;
    });
    t = std::clamp(t, 0.0f, 1.0f);
    if (t <= sorted.front().position)
        return sorted.front().color;
    if (t >= sorted.back().position)
        return sorted.back().color;
    for (size_t i = 0; i + 1 < sorted.size(); ++i)
    {
        const GradientStop& a = sorted[i];
        const GradientStop& b = sorted[i + 1];
        if (t >= a.position && t <= b.position)
        {
            const float span = b.position - a.position;
            const float local = span > 1e-6f ? (t - a.position) / span : 0.0f;
            return LerpColor(a.color, b.color, local);
        }
    }
    return sorted.back().color;
}

void Gradient::Sort()
{
    std::sort(stops.begin(), stops.end(), [](const GradientStop& a, const GradientStop& b) {
        return a.position < b.position;
    });
}

Gradient Gradient::Default()
{
    Gradient gradient;
    gradient.stops.push_back({ 0.0f, HexToColor("#181818") });
    gradient.stops.push_back({ 0.45f, HexToColor("#C4A36B") });
    gradient.stops.push_back({ 1.0f, HexToColor("#F2DDB4") });
    return gradient;
}

bool GradientEditor(const char* id, Gradient& gradient, float height)
{
    if (gradient.stops.empty())
        return false;
    ImGui::PushID(id);
    const float s = S();
    const ImGuiStyle& style = ImGui::GetStyle();
    const float bar_height = height * s;
    const float handle_height = 10.0f * s;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    const float total_height = bar_height + handle_height + 4.0f * s;

    ImGui::InvisibleButton("##gradient", ImVec2(width, total_height));
    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    const ImVec2 bar_min(pos.x, pos.y);
    const ImVec2 bar_max(pos.x + width, pos.y + bar_height);
    const int segments = std::max(8, static_cast<int>(width / 4.0f));
    for (int i = 0; i < segments; ++i)
    {
        const float t0 = static_cast<float>(i) / segments;
        const float t1 = static_cast<float>(i + 1) / segments;
        const ImVec2 a(bar_min.x + width * t0, bar_min.y);
        const ImVec2 b(bar_min.x + width * t1 + 1.0f, bar_max.y);
        draw->AddRectFilledMultiColor(a, b, ImGui::GetColorU32(gradient.Evaluate(t0)),
                                      ImGui::GetColorU32(gradient.Evaluate(t1)),
                                      ImGui::GetColorU32(gradient.Evaluate(t1)),
                                      ImGui::GetColorU32(gradient.Evaluate(t0)));
    }
    draw->AddRect(bar_min, bar_max, U32(ColorToken::Border), style.FrameRounding, 0, 1.0f);

    ImGuiID state_id = ImGui::GetID("##gradient_state");
    ImGuiStorage* storage = ImGui::GetStateStorage();
    int& selected = *storage->GetIntRef(state_id, 0);
    selected = std::clamp(selected, 0, static_cast<int>(gradient.stops.size()) - 1);

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    const float handle_y = bar_max.y + 2.0f * s;
    bool changed = false;

    // Hit test handles (nearest within 8px).
    if (ImGui::IsItemActivated())
    {
        int hit = -1;
        float best = 10.0f * s;
        for (int i = 0; i < static_cast<int>(gradient.stops.size()); ++i)
        {
            const float x = bar_min.x + gradient.stops[i].position * width;
            const float distance = std::fabs(mouse.x - x);
            if (distance < best && mouse.y >= bar_min.y - 2.0f * s)
            {
                best = distance;
                hit = i;
            }
        }
        if (hit >= 0)
            selected = hit;
    }

    if (active && gradient.stops.size() > 1)
    {
        GradientStop& stop = gradient.stops[static_cast<size_t>(selected)];
        const float new_position = std::clamp((mouse.x - bar_min.x) / width, 0.0f, 1.0f);
        if (new_position != stop.position)
        {
            stop.position = new_position;
            changed = true;
        }
    }

    if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        GradientStop stop;
        stop.position = std::clamp((mouse.x - bar_min.x) / width, 0.0f, 1.0f);
        stop.color = gradient.Evaluate(stop.position);
        gradient.stops.push_back(stop);
        selected = static_cast<int>(gradient.stops.size()) - 1;
        changed = true;
    }
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && gradient.stops.size() > 2)
    {
        float best = 10.0f * s;
        int hit = -1;
        for (int i = 0; i < static_cast<int>(gradient.stops.size()); ++i)
        {
            const float x = bar_min.x + gradient.stops[i].position * width;
            const float distance = std::fabs(mouse.x - x);
            if (distance < best)
            {
                best = distance;
                hit = i;
            }
        }
        if (hit >= 0)
        {
            gradient.stops.erase(gradient.stops.begin() + hit);
            selected = std::clamp(selected, 0, static_cast<int>(gradient.stops.size()) - 1);
            changed = true;
        }
    }

    for (int i = 0; i < static_cast<int>(gradient.stops.size()); ++i)
    {
        const GradientStop& stop = gradient.stops[static_cast<size_t>(i)];
        const float x = bar_min.x + stop.position * width;
        const ImU32 outline = i == selected ? U32(ColorToken::AccentHovered) : U32(ColorToken::BorderStrong);
        const ImVec2 top(x, handle_y);
        draw->AddTriangleFilled(top, ImVec2(x - 4.0f * s, handle_y + handle_height), ImVec2(x + 4.0f * s, handle_y + handle_height), outline);
        draw->AddRectFilled(ImVec2(x - 4.0f * s, handle_y + handle_height), ImVec2(x + 4.0f * s, handle_y + handle_height + 4.0f * s),
                            ImGui::GetColorU32(stop.color));
    }

    if (hovered)
        draw->AddRect(bar_min, bar_max, U32(ColorToken::Accent, 0.55f), style.FrameRounding, 0, 1.0f);

    ImGui::Dummy(ImVec2(0.0f, 2.0f * s));
    const ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf;
    ImGui::SetNextItemWidth(std::min(width * 0.5f, 220.0f * s));
    GradientStop& selected_stop = gradient.stops[static_cast<size_t>(selected)];
    if (ImGui::ColorEdit4("##stop_color", &selected_stop.color.x, flags))
        changed = true;
    ImGui::SameLine(0.0f, 8.0f * s);
    ImGui::SetNextItemWidth(std::min(width * 0.35f, 120.0f * s));
    if (ImGui::DragFloat("##stop_pos", &selected_stop.position, 0.005f, 0.0f, 1.0f, "%.2f"))
        changed = true;

    if (changed)
        gradient.Sort();

    ImGui::PopID();
    return changed;
}

// =============================================================================
// Curve
// =============================================================================
float Curve::Evaluate(float x) const
{
    if (points.empty())
        return 0.0f;
    std::vector<ImVec2> sorted = points;
    std::sort(sorted.begin(), sorted.end(), [](const ImVec2& a, const ImVec2& b) { return a.x < b.x; });
    x = std::clamp(x, 0.0f, 1.0f);
    if (x <= sorted.front().x)
        return sorted.front().y;
    if (x >= sorted.back().x)
        return sorted.back().y;
    for (size_t i = 0; i + 1 < sorted.size(); ++i)
    {
        if (x >= sorted[i].x && x <= sorted[i + 1].x)
        {
            const float span = sorted[i + 1].x - sorted[i].x;
            const float t = span > 1e-6f ? (x - sorted[i].x) / span : 0.0f;
            const ImVec2& p0 = sorted[i > 0 ? i - 1 : i];
            const ImVec2& p1 = sorted[i];
            const ImVec2& p2 = sorted[i + 1];
            const ImVec2& p3 = sorted[i + 2 < sorted.size() ? i + 2 : i + 1];
            // Catmull-Rom tangents, clamped to avoid wild overshoot.
            const float m1 = std::clamp((p2.y - p0.y) / std::max(1e-5f, p2.x - p0.x), -4.0f, 4.0f);
            const float m2 = std::clamp((p3.y - p1.y) / std::max(1e-5f, p3.x - p1.x), -4.0f, 4.0f);
            const float t2 = t * t;
            const float t3 = t2 * t;
            const float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
            const float h10 = t3 - 2.0f * t2 + t;
            const float h01 = -2.0f * t3 + 3.0f * t2;
            const float h11 = t3 - t2;
            const float y = h00 * p1.y + h10 * m1 * span + h01 * p2.y + h11 * m2 * span;
            return std::clamp(y, 0.0f, 1.0f);
        }
    }
    return sorted.back().y;
}

Curve Curve::Default()
{
    Curve curve;
    curve.points.push_back(ImVec2(0.0f, 0.0f));
    curve.points.push_back(ImVec2(0.25f, 0.55f));
    curve.points.push_back(ImVec2(0.6f, 0.35f));
    curve.points.push_back(ImVec2(1.0f, 1.0f));
    return curve;
}

bool CurveEditor(const char* id, Curve& curve, const ImVec2& size)
{
    ImGui::PushID(id);
    const float s = S();
    const ImVec2 available = ImGui::GetContentRegionAvail();
    ImVec2 canvas_size = size;
    if (canvas_size.x <= 0.0f)
        canvas_size.x = available.x;
    if (canvas_size.y <= 0.0f)
        canvas_size.y = std::max(80.0f, available.y);

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##curve", canvas_size);
    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 max(pos.x + canvas_size.x, pos.y + canvas_size.y);

    draw->AddRectFilled(pos, max, U32(ColorToken::Background), ImGui::GetStyle().ChildRounding);
    const int grid_x = 8;
    const int grid_y = 4;
    for (int i = 1; i < grid_x; ++i)
    {
        const float x = pos.x + canvas_size.x * static_cast<float>(i) / grid_x;
        draw->AddLine(ImVec2(x, pos.y), ImVec2(x, max.y), U32(ColorToken::Border, 0.45f), 1.0f);
    }
    for (int i = 1; i < grid_y; ++i)
    {
        const float y = pos.y + canvas_size.y * static_cast<float>(i) / grid_y;
        draw->AddLine(ImVec2(pos.x, y), ImVec2(max.x, y), U32(ColorToken::Border, 0.45f), 1.0f);
    }
    draw->AddRect(pos, max, U32(ColorToken::Border), ImGui::GetStyle().ChildRounding, 0, 1.0f);

    auto to_screen = [&](const ImVec2& point) {
        return ImVec2(pos.x + point.x * canvas_size.x, max.y - point.y * canvas_size.y);
    };

    // Curve fill + line.
    const int samples = std::max(32, static_cast<int>(canvas_size.x / 3.0f));
    ImVec2 previous = to_screen(ImVec2(0.0f, curve.Evaluate(0.0f)));
    ImVec2 first = previous;
    for (int i = 1; i <= samples; ++i)
    {
        const float t = static_cast<float>(i) / samples;
        const ImVec2 point = to_screen(ImVec2(t, curve.Evaluate(t)));
        draw->AddLine(previous, point, U32(ColorToken::Accent), 1.8f * s);
        previous = point;
    }
    // Subtle fill below the curve.
    draw->PathClear();
    draw->PathLineTo(first);
    for (int i = 1; i <= samples; ++i)
    {
        const float t = static_cast<float>(i) / samples;
        draw->PathLineTo(to_screen(ImVec2(t, curve.Evaluate(t))));
    }
    draw->PathLineTo(ImVec2(max.x, max.y));
    draw->PathLineTo(ImVec2(pos.x, max.y));
    draw->PathFillConvex(U32(ColorToken::Accent, 0.08f));

    ImGuiStorage* storage = ImGui::GetStateStorage();
    int& selected = *storage->GetIntRef(ImGui::GetID("##curve_selected"), -1);
    if (selected >= static_cast<int>(curve.points.size()))
        selected = -1;

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    if (ImGui::IsItemActivated() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        int hit = -1;
        for (int i = 0; i < static_cast<int>(curve.points.size()); ++i)
        {
            const ImVec2 screen = to_screen(curve.points[static_cast<size_t>(i)]);
            if (std::fabs(mouse.x - screen.x) < 7.0f * s && std::fabs(mouse.y - screen.y) < 7.0f * s)
            {
                hit = i;
                break;
            }
        }
        selected = hit;
    }

    bool changed = false;
    if (active && selected >= 0 && selected < static_cast<int>(curve.points.size()))
    {
        ImVec2& point = curve.points[static_cast<size_t>(selected)];
        const ImVec2 new_point(std::clamp((mouse.x - pos.x) / canvas_size.x, 0.0f, 1.0f),
                               std::clamp((max.y - mouse.y) / canvas_size.y, 0.0f, 1.0f));
        if (new_point.x != point.x || new_point.y != point.y)
        {
            point = new_point;
            changed = true;
        }
    }

    if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        curve.points.push_back(ImVec2(std::clamp((mouse.x - pos.x) / canvas_size.x, 0.0f, 1.0f),
                                      std::clamp((max.y - mouse.y) / canvas_size.y, 0.0f, 1.0f)));
        selected = static_cast<int>(curve.points.size()) - 1;
        changed = true;
    }
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && curve.points.size() > 2)
    {
        for (int i = 0; i < static_cast<int>(curve.points.size()); ++i)
        {
            const ImVec2 screen = to_screen(curve.points[static_cast<size_t>(i)]);
            if (std::fabs(mouse.x - screen.x) < 7.0f * s && std::fabs(mouse.y - screen.y) < 7.0f * s)
            {
                curve.points.erase(curve.points.begin() + i);
                selected = -1;
                changed = true;
                break;
            }
        }
    }

    for (int i = 0; i < static_cast<int>(curve.points.size()); ++i)
    {
        const ImVec2 screen = to_screen(curve.points[static_cast<size_t>(i)]);
        const bool is_selected = i == selected;
        draw->AddCircleFilled(screen, (is_selected ? 5.0f : 4.0f) * s,
                              is_selected ? U32(ColorToken::AccentHovered) : U32(ColorToken::TextPrimary), 16);
        if (is_selected)
            draw->AddCircle(screen, 7.0f * s, U32(ColorToken::Accent, 0.6f), 0, 1.0f);
    }

    if (hovered)
        draw->AddRect(pos, max, U32(ColorToken::Accent, 0.55f), ImGui::GetStyle().ChildRounding, 0, 1.0f);

    ImGui::PopID();
    return changed;
}

} // namespace AmberUI
