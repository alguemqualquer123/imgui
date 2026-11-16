// Amber UI - Built-in vector icons
// SPDX-License-Identifier: MIT
#include "AmberUI/Icons.h"

#include <imgui.h>

#include <cmath>

namespace AmberUI
{
namespace
{
struct IconCanvas
{
    ImDrawList* draw = nullptr;
    ImVec2 center;
    float s = 8.0f;
    ImU32 color = IM_COL32_WHITE;
    float thickness = 1.5f;

    ImVec2 P(float x, float y) const { return ImVec2(center.x + x * s, center.y + y * s); }
    void Line(float x0, float y0, float x1, float y1) const
    {
        draw->AddLine(P(x0, y0), P(x1, y1), color, thickness);
    }
    void Polyline(const ImVec2* points, int count, bool closed = false) const
    {
        ImVec2 scaled[16];
        for (int i = 0; i < count && i < 16; ++i)
            scaled[i] = P(points[i].x, points[i].y);
        draw->AddPolyline(scaled, count, color, closed ? ImDrawFlags_Closed : 0, thickness);
    }
    void Rect(float x0, float y0, float x1, float y1, float rounding = 0.0f, bool filled = false) const
    {
        if (filled)
            draw->AddRectFilled(P(x0, y0), P(x1, y1), color, rounding * s);
        else
            draw->AddRect(P(x0, y0), P(x1, y1), color, rounding * s, 0, thickness);
    }
    void Circle(float cx, float cy, float radius, bool filled = false) const
    {
        if (filled)
            draw->AddCircleFilled(P(cx, cy), radius * s, color, 20);
        else
            draw->AddCircle(P(cx, cy), radius * s, color, 20, thickness);
    }
    void Arc(float cx, float cy, float radius, float min_angle, float max_angle) const
    {
        draw->PathClear();
        draw->PathArcTo(P(cx, cy), radius * s, min_angle, max_angle, 20);
        draw->PathStroke(color, 0, thickness);
    }
};
} // namespace

void DrawIcon(ImDrawList* draw_list, Icon icon, const ImVec2& center, float size, ImU32 color, float thickness)
{
    if (!draw_list || icon == Icon::None)
        return;
    IconCanvas c;
    c.draw = draw_list;
    c.center = center;
    c.s = size * 0.5f;
    c.color = color;
    c.thickness = thickness;

    switch (icon)
    {
    case Icon::Folder:
    case Icon::FolderOpen:
    {
        const ImVec2 points[] = { ImVec2(-0.9f, -0.55f), ImVec2(-0.2f, -0.55f), ImVec2(0.0f, -0.3f),
                                  ImVec2(0.9f, -0.3f), ImVec2(0.9f, 0.6f), ImVec2(-0.9f, 0.6f) };
        c.Polyline(points, 6, true);
        break;
    }
    case Icon::File:
        c.Line(-0.6f, -0.85f, 0.25f, -0.85f);
        c.Line(0.25f, -0.85f, 0.65f, -0.45f);
        c.Line(0.65f, -0.45f, 0.65f, 0.85f);
        c.Line(0.65f, 0.85f, -0.6f, 0.85f);
        c.Line(-0.6f, 0.85f, -0.6f, -0.85f);
        c.Line(0.25f, -0.85f, 0.25f, -0.45f);
        c.Line(0.25f, -0.45f, 0.65f, -0.45f);
        break;
    case Icon::Image:
        c.Rect(-0.85f, -0.7f, 0.85f, 0.7f, 0.1f);
        c.Circle(0.35f, -0.3f, 0.16f, true);
        c.Line(-0.8f, 0.6f, -0.1f, -0.15f);
        c.Line(-0.1f, -0.15f, 0.25f, 0.25f);
        c.Line(0.25f, 0.25f, 0.55f, -0.05f);
        c.Line(0.55f, -0.05f, 0.85f, 0.35f);
        break;
    case Icon::Code:
        c.Line(-0.15f, -0.75f, -0.55f, 0.0f);
        c.Line(-0.55f, 0.0f, -0.15f, 0.75f);
        c.Line(0.15f, -0.75f, 0.55f, 0.0f);
        c.Line(0.55f, 0.0f, 0.15f, 0.75f);
        break;
    case Icon::Settings:
    {
        c.Circle(0.0f, 0.0f, 0.34f);
        for (int i = 0; i < 8; ++i)
        {
            const float angle = static_cast<float>(i) * 3.14159265f * 0.25f;
            c.Line(std::cos(angle) * 0.55f, std::sin(angle) * 0.55f, std::cos(angle) * 0.85f,
                   std::sin(angle) * 0.85f);
        }
        break;
    }
    case Icon::Search:
        c.Circle(-0.15f, -0.15f, 0.55f);
        c.Line(0.25f, 0.25f, 0.8f, 0.8f);
        break;
    case Icon::Close:
        c.Line(-0.6f, -0.6f, 0.6f, 0.6f);
        c.Line(0.6f, -0.6f, -0.6f, 0.6f);
        break;
    case Icon::Plus:
        c.Line(-0.65f, 0.0f, 0.65f, 0.0f);
        c.Line(0.0f, -0.65f, 0.0f, 0.65f);
        break;
    case Icon::Minus:
        c.Line(-0.65f, 0.0f, 0.65f, 0.0f);
        break;
    case Icon::Check:
        c.Line(-0.65f, 0.0f, -0.15f, 0.5f);
        c.Line(-0.15f, 0.5f, 0.7f, -0.55f);
        break;
    case Icon::ChevronDown:
        c.Line(-0.55f, -0.3f, 0.0f, 0.3f);
        c.Line(0.0f, 0.3f, 0.55f, -0.3f);
        break;
    case Icon::ChevronRight:
        c.Line(-0.3f, -0.55f, 0.3f, 0.0f);
        c.Line(0.3f, 0.0f, -0.3f, 0.55f);
        break;
    case Icon::ArrowLeft:
    case Icon::ArrowRight:
    {
        const float dir = icon == Icon::ArrowLeft ? -1.0f : 1.0f;
        c.Line(-0.8f * dir, 0.0f, 0.8f * dir, 0.0f);
        c.Line(0.8f * dir, 0.0f, 0.25f * dir, -0.5f);
        c.Line(0.8f * dir, 0.0f, 0.25f * dir, 0.5f);
        break;
    }
    case Icon::Play:
        draw_list->AddTriangleFilled(c.P(-0.45f, -0.65f), c.P(-0.45f, 0.65f), c.P(0.65f, 0.0f), color);
        break;
    case Icon::Pause:
        c.Rect(-0.55f, -0.65f, -0.15f, 0.65f, 0.05f, true);
        c.Rect(0.15f, -0.65f, 0.55f, 0.65f, 0.05f, true);
        break;
    case Icon::Stop:
        c.Rect(-0.55f, -0.55f, 0.55f, 0.55f, 0.08f, true);
        break;
    case Icon::SkipBack:
        draw_list->AddTriangleFilled(c.P(-0.65f, -0.55f), c.P(-0.65f, 0.55f), c.P(0.35f, 0.0f), color);
        c.Line(0.55f, -0.6f, 0.55f, 0.6f);
        break;
    case Icon::SkipForward:
        draw_list->AddTriangleFilled(c.P(0.65f, -0.55f), c.P(0.65f, 0.55f), c.P(-0.35f, 0.0f), color);
        c.Line(-0.55f, -0.6f, -0.55f, 0.6f);
        break;
    case Icon::Save:
        c.Rect(-0.75f, -0.75f, 0.75f, 0.75f, 0.1f);
        c.Rect(-0.4f, -0.75f, 0.35f, -0.25f);
        c.Rect(-0.55f, 0.15f, 0.55f, 0.75f);
        break;
    case Icon::Open:
    case Icon::Undo:
    case Icon::Redo:
    {
        const float dir = icon == Icon::Redo ? -1.0f : 1.0f;
        c.Arc(0.0f, 0.1f, 0.6f, dir > 0 ? 0.2f : 3.3f, dir > 0 ? 5.6f : 2.4f);
        c.Line(dir * 0.6f, 0.45f, dir * 0.15f, 0.55f);
        c.Line(dir * 0.6f, 0.45f, dir * 0.5f, 0.0f);
        break;
    }
    case Icon::Grid:
        c.Rect(-0.8f, -0.8f, -0.1f, -0.1f, 0.08f);
        c.Rect(0.1f, -0.8f, 0.8f, -0.1f, 0.08f);
        c.Rect(-0.8f, 0.1f, -0.1f, 0.8f, 0.08f);
        c.Rect(0.1f, 0.1f, 0.8f, 0.8f, 0.08f);
        break;
    case Icon::Layers:
        for (int i = 0; i < 3; ++i)
        {
            const float offset = static_cast<float>(i) * 0.28f - 0.28f;
            const ImVec2 points[] = { ImVec2(0.0f, -0.55f + offset), ImVec2(0.75f, -0.15f + offset),
                                      ImVec2(0.0f, 0.25f + offset), ImVec2(-0.75f, -0.15f + offset) };
            c.Polyline(points, 4, true);
        }
        break;
    case Icon::Cube:
    {
        const ImVec2 points[] = { ImVec2(0.0f, -0.85f), ImVec2(0.75f, -0.45f), ImVec2(0.75f, 0.45f),
                                  ImVec2(0.0f, 0.85f), ImVec2(-0.75f, 0.45f), ImVec2(-0.75f, -0.45f) };
        c.Polyline(points, 6, true);
        break;
    }
        c.Line(-0.75f, -0.45f, 0.0f, -0.05f);
        c.Line(0.75f, -0.45f, 0.0f, -0.05f);
        c.Line(0.0f, -0.05f, 0.0f, 0.85f);
        break;
    case Icon::Light:
        c.Circle(0.0f, -0.2f, 0.45f);
        c.Line(-0.25f, 0.35f, -0.25f, 0.55f);
        c.Line(0.25f, 0.35f, 0.25f, 0.55f);
        c.Line(-0.25f, 0.55f, 0.25f, 0.55f);
        c.Line(-0.2f, 0.75f, 0.2f, 0.75f);
        break;
    case Icon::Camera:
        c.Rect(-0.85f, -0.5f, 0.85f, 0.6f, 0.12f);
        c.Circle(0.0f, 0.05f, 0.32f);
        c.Rect(-0.3f, -0.72f, 0.2f, -0.5f, 0.05f);
        break;
    case Icon::Material:
        c.Circle(0.0f, 0.0f, 0.8f);
        c.Arc(0.0f, 0.0f, 0.8f, 3.6f, 5.6f);
        break;
    case Icon::Audio:
        draw_list->AddTriangleFilled(c.P(-0.75f, -0.3f), c.P(-0.75f, 0.3f), c.P(-0.25f, 0.0f), color);
        c.Line(-0.2f, -0.3f, 0.1f, -0.6f);
        c.Rect(-0.25f, -0.6f, 0.1f, 0.6f, 0.05f);
        c.Arc(0.2f, 0.0f, 0.45f, -0.9f, 0.9f);
        c.Arc(0.2f, 0.0f, 0.75f, -0.9f, 0.9f);
        break;
    case Icon::Video:
        c.Rect(-0.85f, -0.6f, 0.85f, 0.6f, 0.1f);
        for (int i = 0; i < 3; ++i)
        {
            const float x = -0.6f + static_cast<float>(i) * 0.6f;
            c.Line(x, -0.6f, x, -0.35f);
            c.Line(x, 0.35f, x, 0.6f);
        }
        break;
    case Icon::Link:
        c.Rect(-0.85f, -0.25f, 0.15f, 0.25f, 0.25f);
        c.Rect(-0.15f, -0.25f, 0.85f, 0.25f, 0.25f);
        break;
    case Icon::Node:
        c.Rect(-0.85f, -0.3f, -0.15f, 0.3f, 0.08f);
        c.Rect(0.15f, -0.3f, 0.85f, 0.3f, 0.08f);
        c.Line(-0.15f, 0.0f, 0.15f, 0.0f);
        break;
    case Icon::Timeline:
        c.Line(-0.85f, 0.0f, 0.85f, 0.0f);
        c.Line(-0.5f, -0.25f, -0.5f, 0.25f);
        c.Line(0.0f, -0.35f, 0.0f, 0.35f);
        c.Line(0.5f, -0.25f, 0.5f, 0.25f);
        break;
    case Icon::Console:
        c.Rect(-0.85f, -0.7f, 0.85f, 0.7f, 0.1f);
        c.Line(-0.55f, -0.2f, -0.15f, 0.1f);
        c.Line(-0.15f, 0.1f, -0.55f, 0.4f);
        c.Line(0.05f, 0.4f, 0.55f, 0.4f);
        break;
    case Icon::Warning:
        draw_list->AddTriangleFilled(c.P(0.0f, -0.75f), c.P(0.85f, 0.7f), c.P(-0.85f, 0.7f), color);
        break;
    case Icon::Error:
        c.Circle(0.0f, 0.0f, 0.8f);
        c.Line(-0.35f, -0.35f, 0.35f, 0.35f);
        c.Line(0.35f, -0.35f, -0.35f, 0.35f);
        break;
    case Icon::Info:
        c.Circle(0.0f, 0.0f, 0.8f);
        c.Line(0.0f, -0.05f, 0.0f, 0.45f);
        c.Circle(0.0f, -0.42f, 0.08f, true);
        break;
    case Icon::Success:
        c.Circle(0.0f, 0.0f, 0.8f);
        c.Line(-0.38f, 0.0f, -0.05f, 0.35f);
        c.Line(-0.05f, 0.35f, 0.42f, -0.35f);
        break;
    case Icon::Eye:
        draw_list->PathClear();
        draw_list->PathLineTo(c.P(-0.9f, 0.0f));
        draw_list->PathBezierCubicCurveTo(c.P(-0.45f, -0.8f), c.P(0.45f, -0.8f), c.P(0.9f, 0.0f));
        draw_list->PathBezierCubicCurveTo(c.P(0.45f, 0.8f), c.P(-0.45f, 0.8f), c.P(-0.9f, 0.0f));
        draw_list->PathStroke(color, ImDrawFlags_Closed, thickness);
        c.Circle(0.0f, 0.0f, 0.28f, true);
        break;
    case Icon::Lock:
        c.Rect(-0.6f, -0.1f, 0.6f, 0.7f, 0.12f);
        c.Arc(0.0f, -0.15f, 0.35f, 3.14159f, 6.28318f);
        break;
    case Icon::Trash:
        c.Line(-0.75f, -0.55f, 0.75f, -0.55f);
        c.Line(-0.25f, -0.55f, -0.15f, -0.8f);
        c.Line(0.25f, -0.55f, 0.15f, -0.8f);
        c.Line(-0.1f, -0.8f, 0.1f, -0.8f);
        {
            const ImVec2 points[] = { ImVec2(-0.55f, -0.5f), ImVec2(-0.45f, 0.8f), ImVec2(0.45f, 0.8f),
                                      ImVec2(0.55f, -0.5f) };
            c.Polyline(points, 4);
        }
        c.Line(-0.2f, -0.25f, -0.2f, 0.55f);
        c.Line(0.2f, -0.25f, 0.2f, 0.55f);
        break;
    case Icon::Copy:
        c.Rect(-0.8f, -0.8f, 0.25f, 0.25f, 0.08f);
        c.Rect(-0.25f, -0.25f, 0.8f, 0.8f, 0.08f);
        break;
    case Icon::Filter:
    {
        const ImVec2 points[] = { ImVec2(-0.8f, -0.6f), ImVec2(0.8f, -0.6f), ImVec2(0.15f, 0.05f),
                                  ImVec2(0.15f, 0.7f), ImVec2(-0.15f, 0.55f), ImVec2(-0.15f, 0.05f) };
        c.Polyline(points, 6, true);
        break;
    }
    case Icon::Sun:
        c.Circle(0.0f, 0.0f, 0.35f);
        for (int i = 0; i < 8; ++i)
        {
            const float angle = static_cast<float>(i) * 3.14159265f * 0.25f;
            c.Line(std::cos(angle) * 0.55f, std::sin(angle) * 0.55f, std::cos(angle) * 0.85f,
                   std::sin(angle) * 0.85f);
        }
        break;
    case Icon::Moon:
        c.Arc(0.0f, 0.0f, 0.75f, 0.7f, 5.6f);
        c.Arc(0.28f, -0.1f, 0.55f, 1.6f, 4.6f);
        break;
    case Icon::Grip:
        for (int y = -1; y <= 1; ++y)
            for (int x = -1; x <= 1; x += 2)
                c.Circle(static_cast<float>(x) * 0.35f, static_cast<float>(y) * 0.45f, 0.1f, true);
        break;
    case Icon::Dot:
        c.Circle(0.0f, 0.0f, 0.35f, true);
        break;
    default:
        break;
    }
}

void IconImage(Icon icon, float size, ImU32 color)
{
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(size, size));
    const ImVec2 center(pos.x + size * 0.5f, pos.y + size * 0.5f);
    DrawIcon(ImGui::GetWindowDrawList(), icon, center, size, color);
}

const char* IconName(Icon icon)
{
    switch (icon)
    {
    case Icon::Folder: return "Folder";
    case Icon::FolderOpen: return "FolderOpen";
    case Icon::File: return "File";
    case Icon::Image: return "Image";
    case Icon::Code: return "Code";
    case Icon::Settings: return "Settings";
    case Icon::Search: return "Search";
    case Icon::Close: return "Close";
    case Icon::Plus: return "Plus";
    case Icon::Minus: return "Minus";
    case Icon::Check: return "Check";
    case Icon::ChevronDown: return "ChevronDown";
    case Icon::ChevronRight: return "ChevronRight";
    case Icon::ArrowLeft: return "ArrowLeft";
    case Icon::ArrowRight: return "ArrowRight";
    case Icon::Play: return "Play";
    case Icon::Pause: return "Pause";
    case Icon::Stop: return "Stop";
    case Icon::SkipBack: return "SkipBack";
    case Icon::SkipForward: return "SkipForward";
    case Icon::Save: return "Save";
    case Icon::Open: return "Open";
    case Icon::Undo: return "Undo";
    case Icon::Redo: return "Redo";
    case Icon::Grid: return "Grid";
    case Icon::Layers: return "Layers";
    case Icon::Cube: return "Cube";
    case Icon::Light: return "Light";
    case Icon::Camera: return "Camera";
    case Icon::Material: return "Material";
    case Icon::Audio: return "Audio";
    case Icon::Video: return "Video";
    case Icon::Link: return "Link";
    case Icon::Node: return "Node";
    case Icon::Timeline: return "Timeline";
    case Icon::Console: return "Console";
    case Icon::Warning: return "Warning";
    case Icon::Error: return "Error";
    case Icon::Info: return "Info";
    case Icon::Success: return "Success";
    case Icon::Eye: return "Eye";
    case Icon::Lock: return "Lock";
    case Icon::Trash: return "Trash";
    case Icon::Copy: return "Copy";
    case Icon::Filter: return "Filter";
    case Icon::Sun: return "Sun";
    case Icon::Moon: return "Moon";
    case Icon::Grip: return "Grip";
    case Icon::Dot: return "Dot";
    default: return "None";
    }
}

} // namespace AmberUI
