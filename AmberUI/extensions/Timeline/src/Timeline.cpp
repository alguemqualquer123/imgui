// Amber UI - Timeline extension
// SPDX-License-Identifier: MIT
#include "AmberUI/Timeline.h"

#include "AmberUI/Animation.h"
#include "AmberUI/Icons.h"
#include "AmberUI/ThemeManager.h"
#include "AmberUI/Widgets.h"

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
const Theme* ThemeForTimeline()
{
    return ThemeManager::Get().ActiveTheme();
}

float S()
{
    const Theme* theme = ThemeForTimeline();
    return theme ? theme->MetricScale() : 1.0f;
}

ImU32 U32(ColorToken token, float alpha = 1.0f)
{
    ImVec4 color = ThemeForTimeline() ? ThemeForTimeline()->Color(token) : ImVec4(0.7f, 0.7f, 0.7f, 1);
    color.w *= alpha;
    return ImGui::GetColorU32(color);
}

ImU32 ColorU32(const ImVec4& color, float alpha = 1.0f)
{
    ImVec4 copy = color;
    copy.w *= alpha;
    return ImGui::GetColorU32(copy);
}

ImVec4 TrackColor(const TimelineTrack& track, int index)
{
    if (track.color.w > 0.0f)
        return track.color;
    const ImVec4 palette[] = { HexToColor("#C4A36B"), HexToColor("#5896BB"), HexToColor("#68976B"),
                               HexToColor("#9B7BB0"), HexToColor("#BC5555"), HexToColor("#5E9E9E") };
    return palette[static_cast<size_t>(index) % (sizeof(palette) / sizeof(palette[0]))];
}

double SnapTime(const TimelineState& state, double time)
{
    if (!state.snapping || state.snap_interval <= 0.0)
        return time;
    return std::round(time / state.snap_interval) * state.snap_interval;
}

void FormatTime(double time, char* buffer, size_t size)
{
    const int minutes = static_cast<int>(time / 60.0);
    const double seconds = time - minutes * 60.0;
    std::snprintf(buffer, size, "%02d:%06.3f", minutes, seconds);
}

void NiceStep(double span, float pixels, double& out_step, int& out_subdivisions)
{
    const double target = span * 80.0 / std::max(1.0, static_cast<double>(pixels));
    const double exponent = std::pow(10.0, std::floor(std::log10(std::max(target, 1e-9))));
    const double normalized = target / exponent;
    double step = 1.0;
    int subdivisions = 4;
    if (normalized <= 1.0) { step = 1.0; subdivisions = 5; }
    else if (normalized <= 2.0) { step = 2.0; subdivisions = 4; }
    else if (normalized <= 5.0) { step = 5.0; subdivisions = 5; }
    else { step = 10.0; subdivisions = 5; }
    out_step = step * exponent;
    out_subdivisions = subdivisions;
}
} // namespace

TimelineTrack& AddTrack(TimelineState& state, const char* name)
{
    TimelineTrack track;
    track.name = name ? name : "Track";
    state.tracks.push_back(std::move(track));
    if (state.selected_track < 0)
        state.selected_track = static_cast<int>(state.tracks.size()) - 1;
    return state.tracks.back();
}

void AddKeyframe(TimelineTrack& track, double time, float value)
{
    for (TimelineKeyframe& key : track.keyframes)
    {
        if (std::fabs(key.time - time) < 1e-6)
        {
            key.value = std::clamp(value, 0.0f, 1.0f);
            return;
        }
    }
    TimelineKeyframe key;
    key.time = time;
    key.value = std::clamp(value, 0.0f, 1.0f);
    track.keyframes.push_back(key);
    SortKeyframes(track);
}

void SortKeyframes(TimelineTrack& track)
{
    std::sort(track.keyframes.begin(), track.keyframes.end(),
              [](const TimelineKeyframe& a, const TimelineKeyframe& b) { return a.time < b.time; });
}

TimelineMarker& AddMarker(TimelineState& state, double time, const char* name)
{
    TimelineMarker marker;
    marker.time = time;
    marker.name = name ? name : "Marker";
    state.markers.push_back(marker);
    return state.markers.back();
}

bool Timeline(const char* id, TimelineState& state, const ImVec2& size)
{
    ImGui::PushID(id);
    const float s = S();
    bool edited = false;

    // ------------------------------------------------------------------
    // Playback controls
    // ------------------------------------------------------------------
    if (IconButton(Icon::SkipBack, "Go to start", ImVec2(26.0f * s, 24.0f * s)))
    {
        state.playhead = 0.0;
        state.playing = false;
        edited = true;
    }
    ImGui::SameLine();
    if (IconButton(state.playing ? Icon::Pause : Icon::Play, state.playing ? "Pause" : "Play",
                   ImVec2(26.0f * s, 24.0f * s)))
        state.playing = !state.playing;
    ImGui::SameLine();
    if (IconButton(Icon::Stop, "Stop", ImVec2(26.0f * s, 24.0f * s)))
    {
        state.playing = false;
        state.playhead = 0.0;
        edited = true;
    }
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    char current[32];
    char total[32];
    FormatTime(state.playhead, current, sizeof(current));
    FormatTime(state.duration, total, sizeof(total));
    ImGui::PushStyleColor(ImGuiCol_Text, ThemeForTimeline() ? ThemeForTimeline()->Color(ColorToken::TextSecondary) : ImVec4(0.7f, 0.7f, 0.7f, 1));
    ImGui::Text("%s / %s", current, total);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (ImGui::Checkbox("Snap", &state.snapping))
        edited = true;
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.0f * s);
    if (ImGui::DragScalar("##snap_interval", ImGuiDataType_Double, &state.snap_interval, 0.01f, nullptr, nullptr, "%.2f"))
        state.snap_interval = std::clamp(state.snap_interval, 0.001, 10.0);
    ImGui::SameLine();
    ImGui::Checkbox("Loop", &state.loop);
    ImGui::SameLine();
    ImGui::Checkbox("Curves", &state.show_curves);

    if (state.playing)
    {
        const double advance = ImGui::GetIO().DeltaTime * static_cast<double>(state.playback_speed);
        state.playhead += advance;
        if (state.playhead >= state.duration)
        {
            if (state.loop && state.duration > 0.0)
                state.playhead = std::fmod(state.playhead, state.duration);
            else
            {
                state.playhead = state.duration;
                state.playing = false;
            }
        }
    }

    // ------------------------------------------------------------------
    // Geometry
    // ------------------------------------------------------------------
    const ImVec2 area_pos = ImGui::GetCursorScreenPos();
    float total_width = size.x;
    if (total_width <= 0.0f)
        total_width = ImGui::GetContentRegionAvail().x;
    else if (total_width < 0.0f)
        total_width = ImGui::GetContentRegionAvail().x + total_width;
    const float label_width = std::min(state.label_width * s, total_width * 0.5f);
    const float time_width = std::max(40.0f, total_width - label_width);
    const float ruler_height = 22.0f * s;
    const double span = std::max(0.01, state.view_end - state.view_start);
    auto time_to_x = [&](double time) {
        return area_pos.x + label_width + static_cast<float>((time - state.view_start) / span) * time_width;
    };
    auto x_to_time = [&](float x) {
        return state.view_start + (static_cast<double>(x - area_pos.x - label_width) / time_width) * span;
    };

    // ------------------------------------------------------------------
    // Ruler
    // ------------------------------------------------------------------
    ImGui::SetCursorScreenPos(area_pos);
    ImGui::InvisibleButton("##ruler", ImVec2(total_width, ruler_height),
                           ImGuiButtonFlags_MouseButtonLeft);
    const bool ruler_hovered = ImGui::IsItemHovered();
    const bool ruler_active = ImGui::IsItemActive();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(area_pos, ImVec2(area_pos.x + total_width, area_pos.y + ruler_height),
                        U32(ColorToken::HeaderBackground));
    draw->AddLine(ImVec2(area_pos.x, area_pos.y + ruler_height), ImVec2(area_pos.x + total_width, area_pos.y + ruler_height),
                  U32(ColorToken::Border));

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    const double mouse_time = SnapTime(state, std::clamp(x_to_time(mouse.x), 0.0, state.duration));

    if (ruler_active && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        const double time = std::clamp(x_to_time(mouse.x), 0.0, state.duration);
        if (ImGui::GetIO().KeyShift)
        {
            if (!state.selection_active)
            {
                state.selection_start = state.playhead;
                state.selection_end = state.playhead;
                state.selection_active = true;
            }
            state.selection_end = time;
            edited = true;
        }
        else
        {
            state.playhead = SnapTime(state, time);
            state.selection_active = false;
        }
    }
    if (ruler_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        const double time = SnapTime(state, std::clamp(x_to_time(mouse.x), 0.0, state.duration));
        AddMarker(state, time, "Marker");
        edited = true;
    }
    if (ruler_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        for (size_t i = 0; i < state.markers.size(); ++i)
        {
            const float x = time_to_x(state.markers[i].time);
            if (std::fabs(mouse.x - x) < 7.0f * s)
            {
                state.markers.erase(state.markers.begin() + static_cast<std::ptrdiff_t>(i));
                edited = true;
                break;
            }
        }
    }

    if (state.selection_active)
    {
        const float x0 = time_to_x(std::min(state.selection_start, state.selection_end));
        const float x1 = time_to_x(std::max(state.selection_start, state.selection_end));
        draw->AddRectFilled(ImVec2(x0, area_pos.y), ImVec2(x1, area_pos.y + ruler_height), U32(ColorToken::Accent, 0.20f));
    }

    double step = 1.0;
    int subdivisions = 5;
    NiceStep(span, time_width, step, subdivisions);
    const double first = std::ceil(state.view_start / step) * step;
    for (double time = first; time <= state.view_end + 1e-9; time += step)
    {
        const float x = time_to_x(time);
        draw->AddLine(ImVec2(x, area_pos.y + ruler_height - 5.0f * s), ImVec2(x, area_pos.y + ruler_height),
                      U32(ColorToken::TextMuted));
        char label[32];
        if (step >= 1.0)
            std::snprintf(label, sizeof(label), "%.0fs", time);
        else if (step >= 0.1)
            std::snprintf(label, sizeof(label), "%.1fs", time);
        else
            std::snprintf(label, sizeof(label), "%.2fs", time);
        draw->AddText(ImVec2(x + 3.0f * s, area_pos.y + 4.0f * s), U32(ColorToken::TextMuted), label);
        for (int sub = 1; sub < subdivisions; ++sub)
        {
            const float sx = time_to_x(time + step * sub / subdivisions);
            draw->AddLine(ImVec2(sx, area_pos.y + ruler_height - 2.0f * s), ImVec2(sx, area_pos.y + ruler_height),
                          U32(ColorToken::BorderStrong));
        }
    }

    for (const TimelineMarker& marker : state.markers)
    {
        if (marker.time < state.view_start - span || marker.time > state.view_end + span)
            continue;
        const float x = time_to_x(marker.time);
        const ImVec4 color = marker.color.w > 0.0f ? marker.color : HexToColor("#D4AB58");
        draw->AddTriangleFilled(ImVec2(x - 5.0f * s, area_pos.y + 2.0f * s), ImVec2(x + 5.0f * s, area_pos.y + 2.0f * s),
                                ImVec2(x, area_pos.y + 9.0f * s), ColorU32(color));
        draw->AddLine(ImVec2(x, area_pos.y + 9.0f * s), ImVec2(x, area_pos.y + ruler_height), ColorU32(color, 0.5f));
        if (!marker.name.empty())
            draw->AddText(ImVec2(x + 6.0f * s, area_pos.y + 2.0f * s), ColorU32(color, 0.9f), marker.name.c_str());
    }

    // Labels column header.
    draw->AddRectFilled(area_pos, ImVec2(area_pos.x + label_width, area_pos.y + ruler_height),
                        U32(ColorToken::PanelBackground));
    draw->AddText(ImVec2(area_pos.x + 8.0f * s, area_pos.y + 4.0f * s), U32(ColorToken::TextSecondary), "Tracks");

    // Playhead in the ruler.
    const float playhead_x = time_to_x(state.playhead);
    draw->AddLine(ImVec2(playhead_x, area_pos.y), ImVec2(playhead_x, area_pos.y + ruler_height),
                  U32(ColorToken::AccentHovered), 1.5f);

    ImGui::SetCursorScreenPos(ImVec2(area_pos.x, area_pos.y + ruler_height));

    // ------------------------------------------------------------------
    // Tracks
    // ------------------------------------------------------------------
    const float body_height = size.y > 0.0f ? std::max(40.0f * s, size.y - ruler_height - ImGui::GetFrameHeightWithSpacing() * 2.0f)
                                             : std::max(60.0f, ImGui::GetContentRegionAvail().y);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::BeginChild("##tracks", ImVec2(total_width, body_height), ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_HorizontalScrollbar))
    {
        const float row_height = std::max(18.0f, state.track_height * s);
        const float content_height = std::max(row_height * static_cast<float>(state.tracks.size()), body_height - 2.0f);
        const ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##track_canvas", ImVec2(total_width, content_height),
                               ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool canvas_hovered = ImGui::IsItemHovered();
        const bool canvas_active = ImGui::IsItemActive();
        ImDrawList* track_draw = ImGui::GetWindowDrawList();
        const ImVec2 canvas_max(canvas_pos.x + total_width, canvas_pos.y + content_height);
        track_draw->PushClipRect(canvas_pos, canvas_max, true);

        // Wheel: zoom horizontally. Shift+wheel: pan.
        if (canvas_hovered && ImGui::GetIO().MouseWheel != 0.0f)
        {
            if (ImGui::GetIO().KeyShift)
            {
                const double shift = -ImGui::GetIO().MouseWheel * span * 0.06;
                state.view_start = std::clamp(state.view_start + shift, 0.0, state.duration);
                state.view_end = std::clamp(state.view_end + shift, 0.0, state.duration);
                if (state.view_end - state.view_start < 0.05)
                    state.view_end = state.view_start + 0.05;
            }
            else
            {
                const double anchor = std::clamp(x_to_time(mouse.x), state.view_start, state.view_end);
                const double factor = std::exp(-ImGui::GetIO().MouseWheel * 0.12);
                double new_span = std::clamp(span * factor, 0.05, std::max(0.05, state.duration * 1.5));
                const double anchor_ratio = (anchor - state.view_start) / span;
                state.view_start = std::clamp(anchor - anchor_ratio * new_span, 0.0, std::max(0.0, state.duration - new_span));
                state.view_end = state.view_start + new_span;
            }
        }

        const int row_from_mouse = static_cast<int>(std::floor((mouse.y - canvas_pos.y) / row_height));
        const bool mouse_in_rows = canvas_hovered && row_from_mouse >= 0 &&
                                   row_from_mouse < static_cast<int>(state.tracks.size()) && mouse.x > canvas_pos.x + label_width;

        // Draw rows.
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(state.tracks.size()), row_height);
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                TimelineTrack& track = state.tracks[static_cast<size_t>(i)];
                const float row_y = canvas_pos.y + row_height * static_cast<float>(i);
                const bool row_selected = state.selected_track == i;
                const ImU32 row_bg = row_selected ? U32(ColorToken::PanelBackground) : U32(ColorToken::Background);
                track_draw->AddRectFilled(ImVec2(canvas_pos.x, row_y), ImVec2(canvas_pos.x + total_width, row_y + row_height), row_bg);
                track_draw->AddLine(ImVec2(canvas_pos.x, row_y + row_height), ImVec2(canvas_pos.x + total_width, row_y + row_height),
                                    U32(ColorToken::Border, 0.6f));
                track_draw->AddLine(ImVec2(canvas_pos.x + label_width, row_y), ImVec2(canvas_pos.x + label_width, row_y + row_height),
                                    U32(ColorToken::Border, 0.6f));
                if (row_selected)
                    track_draw->AddRectFilled(ImVec2(canvas_pos.x, row_y), ImVec2(canvas_pos.x + 3.0f * s, row_y + row_height),
                                              U32(ColorToken::Accent));

                track_draw->AddText(ImVec2(canvas_pos.x + 10.0f * s, row_y + (row_height - ImGui::GetTextLineHeight()) * 0.5f),
                                    U32(track.muted ? ColorToken::TextDisabled : ColorToken::TextSecondary), track.name.c_str());

                // Range highlight inside rows.
                if (state.selection_active)
                {
                    const float x0 = time_to_x(std::min(state.selection_start, state.selection_end));
                    const float x1 = time_to_x(std::max(state.selection_start, state.selection_end));
                    track_draw->AddRectFilled(ImVec2(x0, row_y), ImVec2(x1, row_y + row_height), U32(ColorToken::Accent, 0.08f));
                }

                const ImVec4 color = TrackColor(track, i);
                ImVec2 previous;
                bool has_previous = false;
                if (state.show_curves)
                {
                    for (const TimelineKeyframe& key : track.keyframes)
                    {
                        if (key.time < state.view_start - span || key.time > state.view_end + span)
                            continue;
                        const ImVec2 point(time_to_x(key.time), row_y + row_height - 4.0f * s - key.value * (row_height - 8.0f * s));
                        if (has_previous)
                            track_draw->AddLine(previous, point, ColorU32(color, 0.55f), 1.2f * s);
                        previous = point;
                        has_previous = true;
                    }
                }

                for (int key_index = 0; key_index < static_cast<int>(track.keyframes.size()); ++key_index)
                {
                    const TimelineKeyframe& key = track.keyframes[static_cast<size_t>(key_index)];
                    if (key.time < state.view_start - span || key.time > state.view_end + span)
                        continue;
                    const float x = time_to_x(key.time);
                    const float y = row_y + row_height - 4.0f * s - key.value * (row_height - 8.0f * s);
                    const bool selected = state.selected_track == i && state.selected_keyframe == key_index;
                    const float radius = (selected ? 5.0f : 4.0f) * s;
                    const ImVec2 points[4] = { ImVec2(x, y - radius), ImVec2(x + radius, y), ImVec2(x, y + radius),
                                               ImVec2(x - radius, y) };
                    track_draw->AddConvexPolyFilled(points, 4, selected ? U32(ColorToken::AccentHovered) : ColorU32(color));
                    track_draw->AddPolyline(points, 4, U32(ColorToken::Background), ImDrawFlags_Closed, 1.0f);
                }
            }
        }

        // Playhead over tracks.
        track_draw->AddLine(ImVec2(playhead_x, canvas_pos.y), ImVec2(playhead_x, canvas_max.y),
                            U32(ColorToken::AccentHovered, 0.85f), 1.0f);

        if (canvas_hovered)
            track_draw->AddRect(canvas_pos, canvas_max, U32(ColorToken::Border), 0, 0, 1.0f);
        track_draw->PopClipRect();

        // Interaction.
        ImGuiStorage* storage = ImGui::GetStateStorage();
        int& drag_track = *storage->GetIntRef(ImGui::GetID("##drag_track"), -1);
        int& drag_key = *storage->GetIntRef(ImGui::GetID("##drag_key"), -1);

        if (canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            state.selected_keyframe = -1;
            if (mouse_in_rows)
            {
                state.selected_track = row_from_mouse;
                // Keyframe hit test.
                bool hit = false;
                for (int key_index = 0; key_index < static_cast<int>(state.tracks[static_cast<size_t>(row_from_mouse)].keyframes.size()); ++key_index)
                {
                    const TimelineKeyframe& key = state.tracks[static_cast<size_t>(row_from_mouse)].keyframes[static_cast<size_t>(key_index)];
                    const float x = time_to_x(key.time);
                    const float row_y = canvas_pos.y + row_height * static_cast<float>(row_from_mouse);
                    const float y = row_y + row_height - 4.0f * s - key.value * (row_height - 8.0f * s);
                    if (std::fabs(mouse.x - x) < 7.0f * s && std::fabs(mouse.y - y) < 8.0f * s)
                    {
                        state.selected_keyframe = key_index;
                        drag_track = row_from_mouse;
                        drag_key = key_index;
                        hit = true;
                        break;
                    }
                }
                if (!hit && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    const double time = SnapTime(state, std::clamp(x_to_time(mouse.x), 0.0, state.duration));
                    const float row_y = canvas_pos.y + row_height * static_cast<float>(row_from_mouse);
                    const float value = std::clamp((row_y + row_height - 4.0f * s - mouse.y) / (row_height - 8.0f * s), 0.0f, 1.0f);
                    AddKeyframe(state.tracks[static_cast<size_t>(row_from_mouse)], time, value);
                    edited = true;
                }
            }
        }

        if (canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            if (mouse_in_rows)
            {
                TimelineTrack& track = state.tracks[static_cast<size_t>(row_from_mouse)];
                for (int key_index = 0; key_index < static_cast<int>(track.keyframes.size()); ++key_index)
                {
                    const TimelineKeyframe& key = track.keyframes[static_cast<size_t>(key_index)];
                    const float x = time_to_x(key.time);
                    const float row_y = canvas_pos.y + row_height * static_cast<float>(row_from_mouse);
                    const float y = row_y + row_height - 4.0f * s - key.value * (row_height - 8.0f * s);
                    if (std::fabs(mouse.x - x) < 7.0f * s && std::fabs(mouse.y - y) < 8.0f * s)
                    {
                        track.keyframes.erase(track.keyframes.begin() + key_index);
                        state.selected_keyframe = -1;
                        edited = true;
                        break;
                    }
                }
            }
        }

        if (canvas_active && drag_track >= 0 && drag_key >= 0 && ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
            drag_track < static_cast<int>(state.tracks.size()))
        {
            TimelineTrack& track = state.tracks[static_cast<size_t>(drag_track)];
            if (drag_key < static_cast<int>(track.keyframes.size()))
            {
                TimelineKeyframe& key = track.keyframes[static_cast<size_t>(drag_key)];
                double new_time = std::clamp(x_to_time(mouse.x), 0.0, state.duration);
                if (state.snapping)
                    new_time = SnapTime(state, new_time);
                const float row_y = canvas_pos.y + row_height * static_cast<float>(drag_track);
                const float new_value = std::clamp((row_y + row_height - 4.0f * s - mouse.y) / (row_height - 8.0f * s), 0.0f, 1.0f);
                if (new_time != key.time || new_value != key.value)
                {
                    key.time = new_time;
                    key.value = new_value;
                    edited = true;
                }
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && drag_key >= 0)
        {
            if (drag_track >= 0 && drag_track < static_cast<int>(state.tracks.size()))
                SortKeyframes(state.tracks[static_cast<size_t>(drag_track)]);
            drag_track = -1;
            drag_key = -1;
        }

        if (canvas_hovered && ImGui::IsKeyPressed(ImGuiKey_Delete) && state.selected_track >= 0 &&
            state.selected_keyframe >= 0)
        {
            TimelineTrack& track = state.tracks[static_cast<size_t>(state.selected_track)];
            if (state.selected_keyframe < static_cast<int>(track.keyframes.size()))
            {
                track.keyframes.erase(track.keyframes.begin() + state.selected_keyframe);
                state.selected_keyframe = -1;
                edited = true;
            }
        }

        // Scroll the playhead row into view.
        if (state.selected_track >= 0 && state.selected_track < static_cast<int>(state.tracks.size()))
        {
            const float top = canvas_pos.y + row_height * state.selected_track;
            const float bottom = top + row_height;
            if (bottom > canvas_pos.y + ImGui::GetWindowHeight())
                ImGui::SetScrollY(bottom - ImGui::GetWindowHeight());
            else if (top < canvas_pos.y)
                ImGui::SetScrollY(ImGui::GetScrollY() + top - canvas_pos.y);
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::PopID();
    return edited;
}

} // namespace AmberUI
