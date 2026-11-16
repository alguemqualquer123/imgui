// Amber UI - Node Graph extension
// SPDX-License-Identifier: MIT
#include "AmberUI/NodeGraph.h"

#include "AmberUI/ThemeManager.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace AmberUI
{
namespace
{
const Theme* ThemeForGraph()
{
    return ThemeManager::Get().ActiveTheme();
}

float S()
{
    const Theme* theme = ThemeForGraph();
    return theme ? theme->MetricScale() : 1.0f;
}

ImU32 U32(ColorToken token, float alpha = 1.0f)
{
    ImVec4 color = ThemeForGraph() ? ThemeForGraph()->Color(token) : ImVec4(0.7f, 0.7f, 0.7f, 1);
    color.w *= alpha;
    return ImGui::GetColorU32(color);
}

ImU32 ColorU32(const ImVec4& color, float alpha = 1.0f)
{
    ImVec4 copy = color;
    copy.w *= alpha;
    return ImGui::GetColorU32(copy);
}

ImVec4 ContrastText(const ImVec4& color)
{
    const float luminance = 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
    if (luminance > 0.6f)
        return ImVec4(0.09f, 0.09f, 0.09f, 1.0f);
    return ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
}

struct NodeLayout
{
    ImVec2 min;
    ImVec2 max;
    ImVec2 header_min;
    ImVec2 header_max;
    float row_height = 0.0f;
    int pin_rows = 0;
};

NodeLayout ComputeLayout(const NodeGraphState& state, const GraphNode& node, const ImVec2& canvas_pos)
{
    const float zoom = state.zoom;
    const float width = std::max(node.size.x, 140.0f) * zoom;
    const float header_height = 24.0f * zoom;
    const int rows = std::max(node.inputs.size(), node.outputs.size());
    const float row_height = 18.0f * zoom;
    const float body_height = node.collapsed ? 0.0f : (rows * row_height + 12.0f * zoom);
    const ImVec2 min(canvas_pos.x + (node.position.x + state.pan.x) * zoom,
                     canvas_pos.y + (node.position.y + state.pan.y) * zoom);
    NodeLayout layout;
    layout.min = min;
    layout.max = ImVec2(min.x + width, min.y + header_height + body_height);
    layout.header_min = min;
    layout.header_max = ImVec2(min.x + width, min.y + header_height);
    layout.row_height = row_height;
    layout.pin_rows = rows;
    return layout;
}

ImVec2 InputPinPos(const NodeLayout& layout, int index)
{
    return ImVec2(layout.min.x, layout.header_max.y + 6.0f + layout.row_height * (static_cast<float>(index) + 0.5f));
}

ImVec2 OutputPinPos(const NodeLayout& layout, int index)
{
    return ImVec2(layout.max.x, layout.header_max.y + 6.0f + layout.row_height * (static_cast<float>(index) + 0.5f));
}

bool PointInRect(const ImVec2& point, const ImVec2& min, const ImVec2& max)
{
    return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
}

bool RectsOverlap(const ImVec2& a_min, const ImVec2& a_max, const ImVec2& b_min, const ImVec2& b_max)
{
    return a_min.x < b_max.x && a_max.x > b_min.x && a_min.y < b_max.y && a_max.y > b_min.y;
}

float DistanceToSegmentSquared(const ImVec2& point, const ImVec2& a, const ImVec2& b)
{
    const ImVec2 ab(b.x - a.x, b.y - a.y);
    const float length_squared = ab.x * ab.x + ab.y * ab.y;
    float t = 0.0f;
    if (length_squared > 1e-6f)
        t = ((point.x - a.x) * ab.x + (point.y - a.y) * ab.y) / length_squared;
    t = std::clamp(t, 0.0f, 1.0f);
    const ImVec2 projection(a.x + ab.x * t, a.y + ab.y * t);
    const float dx = point.x - projection.x;
    const float dy = point.y - projection.y;
    return dx * dx + dy * dy;
}

void BezierPoint(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t, ImVec2& out)
{
    const float u = 1.0f - t;
    const float w0 = u * u * u;
    const float w1 = 3.0f * u * u * t;
    const float w2 = 3.0f * u * t * t;
    const float w3 = t * t * t;
    out = ImVec2(p0.x * w0 + p1.x * w1 + p2.x * w2 + p3.x * w3,
                 p0.y * w0 + p1.y * w1 + p2.y * w2 + p3.y * w3);
}

void DrawLink(ImDrawList* draw, const ImVec2& start, const ImVec2& end, ImU32 color, float thickness)
{
    const float horizontal = std::max(40.0f, std::fabs(end.x - start.x) * 0.5f);
    const ImVec2 p1(start.x + horizontal, start.y);
    const ImVec2 p2(end.x - horizontal, end.y);
    draw->AddBezierCubic(start, p1, p2, end, color, thickness);
}

ImVec4 Mix(const ImVec4& a, const ImVec4& b, float t)
{
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t,
                  a.w + (b.w - a.w) * t);
}

void AddTextScaled(ImDrawList* draw, const ImVec2& pos, ImU32 color, const char* text, float font_size)
{
    draw->AddText(ImGui::GetFont(), font_size, pos, color, text);
}

void DrawLinkBevel(ImDrawList* draw, const ImVec2& start, const ImVec2& end, const ImVec4& color, float zoom)
{
    DrawLink(draw, start, end, IM_COL32(0, 0, 0, 110), 5.0f * zoom);
    DrawLink(draw, start, end, ColorU32(Darken(color, 0.35f), 0.95f), 3.2f * zoom);
    DrawLink(draw, start, end, ColorU32(color, 0.9f), 1.7f * zoom);
    DrawLink(draw, start, end, ColorU32(Lighten(color, 0.6f), 0.5f), 0.8f * zoom);
}

GraphNode* NodeByPin(NodeGraphState& state, const ImVec2& canvas_pos, const ImVec2& mouse, bool outputs_only, int& out_pin)
{
    for (int i = static_cast<int>(state.nodes.size()) - 1; i >= 0; --i)
    {
        GraphNode& node = state.nodes[static_cast<size_t>(i)];
        const NodeLayout layout = ComputeLayout(state, node, canvas_pos);
        for (int pin = 0; pin < static_cast<int>(node.outputs.size()); ++pin)
        {
            const ImVec2 center = OutputPinPos(layout, pin);
            if (std::fabs(mouse.x - center.x) < 8.0f * state.zoom && std::fabs(mouse.y - center.y) < 8.0f * state.zoom)
            {
                out_pin = pin;
                return &node;
            }
        }
        if (!outputs_only)
        {
            for (int pin = 0; pin < static_cast<int>(node.inputs.size()); ++pin)
            {
                const ImVec2 center = InputPinPos(layout, pin);
                if (std::fabs(mouse.x - center.x) < 8.0f * state.zoom && std::fabs(mouse.y - center.y) < 8.0f * state.zoom)
                {
                    out_pin = pin;
                    return &node;
                }
            }
        }
    }
    return nullptr;
}

GraphNode* NodeAt(NodeGraphState& state, const ImVec2& canvas_pos, const ImVec2& mouse)
{
    for (int i = static_cast<int>(state.nodes.size()) - 1; i >= 0; --i)
    {
        GraphNode& node = state.nodes[static_cast<size_t>(i)];
        const NodeLayout layout = ComputeLayout(state, node, canvas_pos);
        if (PointInRect(mouse, layout.min, layout.max))
            return &node;
    }
    return nullptr;
}

int LinkAt(NodeGraphState& state, const ImVec2& canvas_pos, const ImVec2& mouse)
{
    for (int i = static_cast<int>(state.links.size()) - 1; i >= 0; --i)
    {
        const GraphLink& link = state.links[static_cast<size_t>(i)];
        GraphNode* from = FindNode(state, link.from_node);
        GraphNode* to = FindNode(state, link.to_node);
        if (!from || !to)
            continue;
        if (link.from_pin >= static_cast<int>(from->outputs.size()) ||
            link.to_pin >= static_cast<int>(to->inputs.size()))
            continue;
        const NodeLayout from_layout = ComputeLayout(state, *from, canvas_pos);
        const NodeLayout to_layout = ComputeLayout(state, *to, canvas_pos);
        const ImVec2 start = OutputPinPos(from_layout, link.from_pin);
        const ImVec2 end = InputPinPos(to_layout, link.to_pin);
        const float horizontal = std::max(40.0f, std::fabs(end.x - start.x) * 0.5f);
        const ImVec2 p1(start.x + horizontal, start.y);
        const ImVec2 p2(end.x - horizontal, end.y);
        ImVec2 previous = start;
        for (int step = 1; step <= 16; ++step)
        {
            ImVec2 point;
            BezierPoint(start, p1, p2, end, static_cast<float>(step) / 16.0f, point);
            if (DistanceToSegmentSquared(mouse, previous, point) < 36.0f)
                return link.id;
            previous = point;
        }
    }
    return 0;
}
} // namespace

const char* NodeCategoryName(NodeCategory category)
{
    switch (category)
    {
    case NodeCategory::Flow: return "Flow";
    case NodeCategory::Function: return "Function";
    case NodeCategory::Input: return "Input";
    case NodeCategory::Output: return "Output";
    case NodeCategory::Animation: return "Animation";
    case NodeCategory::Math: return "Math";
    case NodeCategory::Event: return "Event";
    case NodeCategory::Data: return "Data";
    default: return "Unknown";
    }
}

ImVec4 NodeCategoryColor(NodeCategory category)
{
    switch (category)
    {
    case NodeCategory::Flow: return CategoryColor("flow");
    case NodeCategory::Function: return CategoryColor("function");
    case NodeCategory::Input: return CategoryColor("input");
    case NodeCategory::Output: return CategoryColor("output");
    case NodeCategory::Animation: return CategoryColor("animation");
    case NodeCategory::Math: return CategoryColor("math");
    case NodeCategory::Event: return CategoryColor("event");
    case NodeCategory::Data: return CategoryColor("data");
    default: return CategoryColor(nullptr);
    }
}

GraphNode& AddNode(NodeGraphState& state, const char* title, NodeCategory category, const ImVec2& position)
{
    GraphNode node;
    node.id = state.next_id++;
    node.title = title ? title : "Node";
    node.category = category;
    node.position = position;
    state.nodes.push_back(std::move(node));
    return state.nodes.back();
}

GraphGroup& AddGroup(NodeGraphState& state, const char* title, const ImVec2& position, const ImVec2& size)
{
    GraphGroup group;
    group.id = state.next_id++;
    group.title = title ? title : "Group";
    group.position = position;
    group.size = size;
    state.groups.push_back(std::move(group));
    return state.groups.back();
}

GraphComment& AddComment(NodeGraphState& state, const char* text, const ImVec2& position)
{
    GraphComment comment;
    comment.id = state.next_id++;
    comment.text = text ? text : "Comment";
    comment.position = position;
    state.comments.push_back(std::move(comment));
    return state.comments.back();
}

int AddLink(NodeGraphState& state, int from_node, int from_pin, int to_node, int to_pin)
{
    if (from_node == to_node)
        return 0;
    for (const GraphLink& link : state.links)
    {
        if (link.from_node == from_node && link.from_pin == from_pin && link.to_node == to_node && link.to_pin == to_pin)
            return 0;
    }
    GraphLink link;
    link.id = state.next_id++;
    link.from_node = from_node;
    link.from_pin = from_pin;
    link.to_node = to_node;
    link.to_pin = to_pin;
    state.links.push_back(link);
    return link.id;
}

GraphNode* FindNode(NodeGraphState& state, int id)
{
    for (GraphNode& node : state.nodes)
    {
        if (node.id == id)
            return &node;
    }
    return nullptr;
}

void RemoveNode(NodeGraphState& state, int node_id)
{
    state.nodes.erase(std::remove_if(state.nodes.begin(), state.nodes.end(),
                                     [node_id](const GraphNode& node) { return node.id == node_id; }),
                      state.nodes.end());
    state.links.erase(std::remove_if(state.links.begin(), state.links.end(),
                                     [node_id](const GraphLink& link) {
                                         return link.from_node == node_id || link.to_node == node_id;
                                     }),
                      state.links.end());
    if (state.selected_node == node_id)
        state.selected_node = -1;
}

void ClearSelection(NodeGraphState& state)
{
    for (GraphNode& node : state.nodes)
        node.selected = false;
    for (GraphGroup& group : state.groups)
        group.selected = false;
    state.selected_node = -1;
}

bool NodeGraph(const char* id, NodeGraphState& state, const ImVec2& size)
{
    ImGui::PushID(id);
    const float s = S();
    ImVec2 canvas_size = size;
    const ImVec2 available = ImGui::GetContentRegionAvail();
    if (canvas_size.x <= 0.0f)
        canvas_size.x = available.x;
    if (canvas_size.y <= 0.0f)
        canvas_size.y = std::max(160.0f, available.y);

    const ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    const ImVec2 canvas_max(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y);
    ImGui::InvisibleButton("##graph_canvas", canvas_size,
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight |
                               ImGuiButtonFlags_MouseButtonMiddle);
    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->PushClipRect(canvas_pos, canvas_max, true);

    draw->AddRectFilled(canvas_pos, canvas_max, U32(ColorToken::Background));
    bool changed = false;
    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 mouse = io.MousePos;
    const ImVec2 minimap_size(160.0f * s, 110.0f * s);
    const ImVec2 minimap_max(canvas_max.x - 10.0f * s, canvas_max.y - 10.0f * s);
    const ImVec2 minimap_min(minimap_max.x - minimap_size.x, minimap_max.y - minimap_size.y);
    const bool mouse_over_minimap = state.show_minimap && PointInRect(mouse, minimap_min, minimap_max);

    // ------------------------------------------------------------------
    // Pan / zoom
    // ------------------------------------------------------------------
    if (hovered && io.MouseWheel != 0.0f)
    {
        const float old_zoom = state.zoom;
        state.zoom = std::clamp(state.zoom * std::exp(io.MouseWheel * 0.12f), 0.25f, 2.5f);
        const ImVec2 graph_before((mouse.x - canvas_pos.x) / old_zoom - state.pan.x,
                                  (mouse.y - canvas_pos.y) / old_zoom - state.pan.y);
        state.pan.x = (mouse.x - canvas_pos.x) / state.zoom - graph_before.x;
        state.pan.y = (mouse.y - canvas_pos.y) / state.zoom - graph_before.y;
    }
    if (hovered && (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) ||
                    (io.KeyAlt && ImGui::IsMouseDragging(ImGuiMouseButton_Left))))
    {
        state.pan.x += io.MouseDelta.x / state.zoom;
        state.pan.y += io.MouseDelta.y / state.zoom;
    }

    // ------------------------------------------------------------------
    // Grid
    // ------------------------------------------------------------------
    if (state.show_grid)
    {
        float spacing = state.grid_size * state.zoom;
        while (spacing < 12.0f)
            spacing *= 2.0f;
        const float offset_x = std::fmod(state.pan.x * state.zoom, spacing);
        const float offset_y = std::fmod(state.pan.y * state.zoom, spacing);
        const float major = spacing * 4.0f;
        const float major_offset_x = std::fmod(state.pan.x * state.zoom, major);
        const float major_offset_y = std::fmod(state.pan.y * state.zoom, major);
        const ImU32 dot_color = U32(ColorToken::Border, 0.32f);
        const ImU32 dot_major_color = U32(ColorToken::Border, 0.75f);
        const float dot_radius = std::max(1.0f, 1.1f * s);
        const float dot_major_radius = std::max(1.5f, 1.7f * s);
        for (float x = offset_x; x < canvas_size.x; x += spacing)
            for (float y = offset_y; y < canvas_size.y; y += spacing)
                draw->AddRectFilled(ImVec2(canvas_pos.x + x - dot_radius, canvas_pos.y + y - dot_radius),
                                    ImVec2(canvas_pos.x + x + dot_radius, canvas_pos.y + y + dot_radius), dot_color);
        for (float x = major_offset_x; x < canvas_size.x; x += major)
            for (float y = major_offset_y; y < canvas_size.y; y += major)
                draw->AddCircleFilled(ImVec2(canvas_pos.x + x, canvas_pos.y + y), dot_major_radius, dot_major_color, 8);
    }

    // ------------------------------------------------------------------
    // Groups and comments (behind nodes)
    // ------------------------------------------------------------------
    for (GraphGroup& group : state.groups)
    {
        ImVec4 color = group.color.w > 0.0f ? group.color : HexToColor("#5F7E9E");
        const ImVec2 min(canvas_pos.x + (group.position.x + state.pan.x) * state.zoom,
                         canvas_pos.y + (group.position.y + state.pan.y) * state.zoom);
        const ImVec2 max(min.x + group.size.x * state.zoom, min.y + group.size.y * state.zoom);
        const float rounding = 6.0f * s * state.zoom;
        const float title_height = std::min(max.y - min.y, 22.0f * s * state.zoom);
        draw->AddRectFilled(min, max, ColorU32(color, 0.07f), rounding);
        draw->AddRect(min, max, ColorU32(color, 0.5f), rounding, 0, 1.0f);
        const ImVec2 title_max(max.x, min.y + title_height);
        draw->AddRectFilled(min, title_max, ColorU32(color, 0.20f), rounding, ImDrawFlags_RoundCornersTop);
        draw->AddLine(ImVec2(min.x, title_max.y),
                      ImVec2(max.x, title_max.y), ColorU32(color, 0.35f));
        const float font_size = std::max(6.0f, ImGui::GetFontSize() * state.zoom);
        AddTextScaled(draw, ImVec2(min.x + 8.0f * s * state.zoom, min.y + (title_height - font_size) * 0.5f),
                      ColorU32(color, 0.95f), group.title.c_str(), font_size);
    }
    for (const GraphComment& comment : state.comments)
    {
        ImVec4 color = comment.color.w > 0.0f ? comment.color : HexToColor("#D4AB58");
        const ImVec2 min(canvas_pos.x + (comment.position.x + state.pan.x) * state.zoom,
                         canvas_pos.y + (comment.position.y + state.pan.y) * state.zoom);
        const ImVec2 max(min.x + comment.size.x * state.zoom, min.y + comment.size.y * state.zoom);
        const float rounding = 4.0f * s * state.zoom;
        draw->AddRectFilled(ImVec2(min.x + 2.0f, min.y + 2.0f), ImVec2(max.x + 2.0f, max.y + 2.0f),
                            IM_COL32(0, 0, 0, 40), rounding);
        draw->AddRectFilled(min, max, ColorU32(color, 0.09f), rounding);
        draw->AddRect(min, max, ColorU32(color, 0.5f), rounding);
        const float font_size = std::max(6.0f, ImGui::GetFontSize() * std::max(state.zoom, 0.7f));
        draw->AddText(ImGui::GetFont(), font_size, ImVec2(min.x + 8.0f * s, min.y + 8.0f * s),
                      U32(ColorToken::TextSecondary), comment.text.c_str(), nullptr,
                      std::max(20.0f, (comment.size.x - 16.0f) * state.zoom));
    }

    // ------------------------------------------------------------------
    // Links
    // ------------------------------------------------------------------
    int hovered_link = 0;
    if (hovered && state.linking_node <= 0 && !mouse_over_minimap)
        hovered_link = LinkAt(state, canvas_pos, mouse);
    for (const GraphLink& link : state.links)
    {
        GraphNode* from = FindNode(state, link.from_node);
        GraphNode* to = FindNode(state, link.to_node);
        if (!from || !to)
            continue;
        if (link.from_pin >= static_cast<int>(from->outputs.size()) ||
            link.to_pin >= static_cast<int>(to->inputs.size()))
            continue;
        const NodeLayout from_layout = ComputeLayout(state, *from, canvas_pos);
        const NodeLayout to_layout = ComputeLayout(state, *to, canvas_pos);
        const ImVec2 start = OutputPinPos(from_layout, link.from_pin);
        const ImVec2 end = InputPinPos(to_layout, link.to_pin);
        ImVec4 color = link.color.w > 0.0f ? link.color : NodeCategoryColor(from->category);
        const bool connected_selected = from->selected || to->selected ||
                                        state.selected_node == from->id || state.selected_node == to->id;
        if (connected_selected)
        {
            color = Lighten(color, 0.12f);
            DrawLink(draw, start, end, ColorU32(color, 0.16f), 9.0f * state.zoom);
        }
        else if (link.id == hovered_link)
        {
            DrawLink(draw, start, end, U32(ColorToken::Accent, 0.30f), 7.5f * state.zoom);
        }
        DrawLinkBevel(draw, start, end, color, state.zoom);
    }

    // Link being created.
    if (state.linking_node > 0)
    {
        GraphNode* from = FindNode(state, state.linking_node);
        if (from)
        {
            const NodeLayout layout = ComputeLayout(state, *from, canvas_pos);
            ImVec2 start;
            if (state.linking_output && state.linking_pin < static_cast<int>(from->outputs.size()))
                start = OutputPinPos(layout, state.linking_pin);
            else if (!state.linking_output && state.linking_pin < static_cast<int>(from->inputs.size()))
                start = InputPinPos(layout, state.linking_pin);
            DrawLink(draw, start, mouse, U32(ColorToken::Accent, 0.16f), 7.0f * state.zoom);
            DrawLink(draw, start, mouse, U32(ColorToken::Accent, 0.9f), 2.0f * state.zoom);
            draw->AddCircleFilled(mouse, 5.0f * state.zoom, U32(ColorToken::Background), 16);
            draw->AddCircleFilled(mouse, 3.4f * state.zoom, U32(ColorToken::AccentHovered), 16);
        }
    }

    // ------------------------------------------------------------------
    // Nodes
    // ------------------------------------------------------------------
    for (GraphNode& node : state.nodes)
    {
        const NodeLayout layout = ComputeLayout(state, node, canvas_pos);
        const ImVec4 category = NodeCategoryColor(node.category);
        const bool is_selected = node.selected || state.selected_node == node.id;
        const bool is_hovered = hovered && PointInRect(mouse, layout.min, layout.max);
        const float rounding = 6.0f * s * state.zoom;
        const float font_size = ImGui::GetFontSize() * std::clamp(state.zoom, 0.5f, 2.0f);

        draw->AddRectFilled(ImVec2(layout.min.x, layout.min.y + 5.0f * state.zoom),
                            ImVec2(layout.max.x + 3.0f * state.zoom, layout.max.y + 5.0f * state.zoom),
                            IM_COL32(0, 0, 0, 55), rounding);

        if (is_selected)
        {
            draw->AddRect(layout.min, layout.max, U32(ColorToken::Accent, 0.22f), rounding, 0, 6.0f * state.zoom);
            draw->AddRect(layout.min, layout.max, U32(ColorToken::Accent, 0.35f), rounding, 0, 3.5f * state.zoom);
        }

        draw->AddRectFilled(layout.min, layout.max, U32(ColorToken::PanelBackground), rounding);

        if (!node.collapsed && layout.pin_rows > 1)
        {
            const float rows_top = layout.header_max.y + 6.0f * state.zoom;
            for (int i = 1; i < layout.pin_rows; i += 2)
            {
                const float y0 = rows_top + layout.row_height * static_cast<float>(i);
                const float y1 = std::min(y0 + layout.row_height, layout.max.y);
                draw->AddRectFilled(ImVec2(layout.min.x + 1.0f, y0), ImVec2(layout.max.x - 1.0f, y1),
                                    IM_COL32(0, 0, 0, 12));
            }
        }

        const ImDrawFlags header_flags = node.collapsed ? 0 : ImDrawFlags_RoundCornersTop;
        draw->AddRectFilled(layout.header_min, layout.header_max, ColorU32(category, 0.95f), rounding, header_flags);
        draw->AddRectFilled(ImVec2(layout.min.x + rounding, layout.min.y + 1.0f),
                            ImVec2(layout.max.x - rounding, layout.min.y + 1.0f + std::max(1.0f, s)),
                            ColorU32(Lighten(category, 0.45f), 0.5f));
        draw->AddRectFilled(ImVec2(layout.min.x, layout.header_max.y - 0.5f),
                            ImVec2(layout.max.x, layout.header_max.y), IM_COL32(0, 0, 0, 90));

        if (is_selected)
            draw->AddRect(layout.min, layout.max, U32(ColorToken::Accent), rounding, 0,
                          std::max(1.6f, 2.0f * state.zoom));
        else if (is_hovered)
            draw->AddRect(layout.min, layout.max, U32(ColorToken::Accent, 0.65f), rounding, 0,
                          std::max(1.0f, 1.2f * state.zoom) * 1.35f);
        else
            draw->AddRect(layout.min, layout.max, U32(ColorToken::BorderStrong), rounding, 0,
                          std::max(1.0f, 1.2f * state.zoom));

        if (!node.collapsed)
        {
            const float shadow_h = std::min(3.0f * state.zoom, (layout.max.y - layout.min.y) * 0.5f);
            draw->AddRectFilled(ImVec2(layout.min.x, layout.max.y - shadow_h), layout.max,
                                IM_COL32(0, 0, 0, 26), rounding, ImDrawFlags_RoundCornersBottom);
        }

        const float title_y = layout.min.y + ((layout.header_max.y - layout.min.y) - font_size) * 0.5f;
        AddTextScaled(draw, ImVec2(layout.min.x + 8.0f * s * state.zoom, title_y),
                      ImGui::GetColorU32(ContrastText(category)), node.title.c_str(), font_size);
        const char* category_name = NodeCategoryName(node.category);
        const float chip_font = font_size * 0.8f;
        const ImVec2 title_extent = ImGui::GetFont()->CalcTextSizeA(font_size, FLT_MAX, 0.0f, node.title.c_str());
        const ImVec2 chip_extent = ImGui::GetFont()->CalcTextSizeA(chip_font, FLT_MAX, 0.0f, category_name);
        if (title_extent.x + chip_extent.x + 24.0f * s * state.zoom < layout.max.x - layout.min.x)
            AddTextScaled(draw, ImVec2(layout.max.x - 8.0f * s * state.zoom - chip_extent.x,
                                       layout.min.y + ((layout.header_max.y - layout.min.y) - chip_font) * 0.5f),
                          ColorU32(ContrastText(category), 0.55f), category_name, chip_font);

        if (!node.collapsed)
        {
            for (int i = 0; i < static_cast<int>(node.inputs.size()); ++i)
            {
                const GraphPin& pin = node.inputs[static_cast<size_t>(i)];
                const ImVec2 center = InputPinPos(layout, i);
                const ImVec4 pin_color = pin.color.w > 0.0f ? pin.color : category;
                const bool pin_hot = is_hovered && std::fabs(mouse.x - center.x) < 8.0f * state.zoom &&
                                     std::fabs(mouse.y - center.y) < 8.0f * state.zoom;
                draw->AddCircleFilled(center, 4.6f * state.zoom, U32(ColorToken::Background), 16);
                draw->AddCircleFilled(center, (pin_hot ? 4.1f : 3.3f) * state.zoom,
                                      ColorU32(pin_color, pin_hot ? 1.0f : 0.95f), 16);
                if (pin_hot)
                    draw->AddCircle(center, 6.2f * state.zoom, ColorU32(Lighten(pin_color, 0.4f), 0.55f), 16,
                                    std::max(1.0f, 1.4f * state.zoom));
                else
                    draw->AddCircleFilled(center, 1.1f * state.zoom, IM_COL32(255, 255, 255, 90), 6);
                const float pin_font = font_size * 0.92f;
                AddTextScaled(draw, ImVec2(center.x + 9.0f * s * state.zoom, center.y - pin_font * 0.5f),
                              U32(pin_hot ? ColorToken::TextPrimary : ColorToken::TextSecondary), pin.name.c_str(),
                              pin_font);
            }
            for (int i = 0; i < static_cast<int>(node.outputs.size()); ++i)
            {
                const GraphPin& pin = node.outputs[static_cast<size_t>(i)];
                const ImVec2 center = OutputPinPos(layout, i);
                const ImVec4 pin_color = pin.color.w > 0.0f ? pin.color : category;
                const bool pin_hot = is_hovered && std::fabs(mouse.x - center.x) < 8.0f * state.zoom &&
                                     std::fabs(mouse.y - center.y) < 8.0f * state.zoom;
                draw->AddCircleFilled(center, 4.6f * state.zoom, U32(ColorToken::Background), 16);
                draw->AddCircleFilled(center, (pin_hot ? 4.1f : 3.3f) * state.zoom,
                                      ColorU32(pin_color, pin_hot ? 1.0f : 0.95f), 16);
                if (pin_hot)
                    draw->AddCircle(center, 6.2f * state.zoom, ColorU32(Lighten(pin_color, 0.4f), 0.55f), 16,
                                    std::max(1.0f, 1.4f * state.zoom));
                else
                    draw->AddCircleFilled(center, 1.1f * state.zoom, IM_COL32(255, 255, 255, 90), 6);
                const float pin_font = font_size * 0.92f;
                const ImVec2 text_extent = ImGui::GetFont()->CalcTextSizeA(pin_font, FLT_MAX, 0.0f, pin.name.c_str());
                AddTextScaled(draw, ImVec2(center.x - 9.0f * s * state.zoom - text_extent.x,
                                           center.y - pin_font * 0.5f),
                              U32(pin_hot ? ColorToken::TextPrimary : ColorToken::TextSecondary), pin.name.c_str(),
                              pin_font);
            }
        }
    }

    // ------------------------------------------------------------------
    // Box selection
    // ------------------------------------------------------------------
    if (state.box_selecting)
    {
        const ImVec2 a(std::min(state.box_start.x, mouse.x), std::min(state.box_start.y, mouse.y));
        const ImVec2 b(std::max(state.box_start.x, mouse.x), std::max(state.box_start.y, mouse.y));
        draw->AddRectFilled(a, b, U32(ColorToken::Accent, 0.10f));
        draw->AddRect(a, b, U32(ColorToken::Accent, 0.7f));
    }

    // ------------------------------------------------------------------
    // Minimap
    // ------------------------------------------------------------------
    if (state.show_minimap)
    {
        const float minimap_rounding = 5.0f * s;
        draw->AddRectFilled(ImVec2(minimap_min.x, minimap_min.y + 3.0f * s),
                            ImVec2(minimap_max.x + 2.0f * s, minimap_max.y + 3.0f * s),
                            IM_COL32(0, 0, 0, 60), minimap_rounding);
        draw->AddRectFilled(minimap_min, minimap_max, IM_COL32(10, 10, 10, 205), minimap_rounding);
        draw->AddRect(minimap_min, minimap_max, U32(ColorToken::BorderStrong), minimap_rounding);
        draw->AddRectFilled(ImVec2(minimap_min.x + 5.0f * s, minimap_min.y + 1.0f),
                            ImVec2(minimap_max.x - 5.0f * s, minimap_min.y + 1.0f + std::max(1.0f, s)),
                            IM_COL32(255, 255, 255, 18), 1.0f);

        ImVec2 graph_min(0, 0);
        ImVec2 graph_max(1, 1);
        bool has_nodes = false;
        for (const GraphNode& node : state.nodes)
        {
            const NodeLayout layout = ComputeLayout(state, node, ImVec2(0, 0));
            if (!has_nodes)
            {
                graph_min = layout.min;
                graph_max = layout.max;
                has_nodes = true;
            }
            graph_min.x = std::min(graph_min.x, layout.min.x);
            graph_min.y = std::min(graph_min.y, layout.min.y);
            graph_max.x = std::max(graph_max.x, layout.max.x);
            graph_max.y = std::max(graph_max.y, layout.max.y);
        }
        if (!has_nodes)
        {
            graph_min = ImVec2(-state.pan.x, -state.pan.y);
            graph_max = ImVec2(graph_min.x + canvas_size.x / state.zoom, graph_min.y + canvas_size.y / state.zoom);
        }
        const float margin = 30.0f;
        graph_min.x -= margin;
        graph_min.y -= margin;
        graph_max.x += margin;
        graph_max.y += margin;
        const float scale_x = (minimap_size.x - 8.0f) / std::max(1.0f, graph_max.x - graph_min.x);
        const float scale_y = (minimap_size.y - 8.0f) / std::max(1.0f, graph_max.y - graph_min.y);
        const float scale = std::min(scale_x, scale_y);
        auto to_minimap = [&](const ImVec2& point) {
            return ImVec2(minimap_min.x + 4.0f + (point.x - graph_min.x) * scale,
                          minimap_min.y + 4.0f + (point.y - graph_min.y) * scale);
        };
        for (const GraphNode& node : state.nodes)
        {
            const NodeLayout layout = ComputeLayout(state, node, ImVec2(0, 0));
            const ImVec2 a = to_minimap(layout.min);
            const ImVec2 b = to_minimap(layout.max);
            draw->AddRectFilled(a, b, ColorU32(NodeCategoryColor(node.category), 0.65f), 1.5f);
        }
        const ImVec2 view_min = to_minimap(ImVec2(-state.pan.x, -state.pan.y));
        const ImVec2 view_max = to_minimap(ImVec2(-state.pan.x + canvas_size.x / state.zoom,
                                                  -state.pan.y + canvas_size.y / state.zoom));
        draw->AddRectFilled(view_min, view_max, U32(ColorToken::Accent, 0.12f), 2.0f);
        draw->AddRect(view_min, view_max, U32(ColorToken::Accent, 0.9f), 2.0f, 0, 1.2f);

        if (mouse_over_minimap && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            const ImVec2 graph_point((mouse.x - minimap_min.x - 4.0f) / scale + graph_min.x,
                                     (mouse.y - minimap_min.y - 4.0f) / scale + graph_min.y);
            state.pan.x = -graph_point.x + (canvas_size.x / state.zoom) * 0.5f;
            state.pan.y = -graph_point.y + (canvas_size.y / state.zoom) * 0.5f;
        }
    }

    // ------------------------------------------------------------------
    // Interaction
    // ------------------------------------------------------------------
    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID context_pos_id = ImGui::GetID("##graph_context_pos");
    ImGuiID context_node_id = ImGui::GetID("##graph_context_node");
    ImGuiID context_link_id = ImGui::GetID("##graph_context_link");

    if (!mouse_over_minimap)
    {
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            int pin = -1;
            GraphNode* pin_node = NodeByPin(state, canvas_pos, mouse, false, pin);
            if (pin_node)
            {
                state.linking_node = pin_node->id;
                state.linking_pin = pin;
                state.linking_output = pin < static_cast<int>(pin_node->outputs.size());
            }
            else
            {
                GraphNode* node = NodeAt(state, canvas_pos, mouse);
                if (node)
                {
                    if (io.KeyCtrl && state.allow_multiselect)
                    {
                        node->selected = !node->selected;
                        state.selected_node = node->selected ? node->id : -1;
                    }
                    else if (!node->selected)
                    {
                        ClearSelection(state);
                        node->selected = true;
                        state.selected_node = node->id;
                    }
                    state.dragging_node = node->id;
                }
                else
                {
                    if (!io.KeyCtrl)
                        ClearSelection(state);
                    state.box_selecting = false;
                    state.box_start = mouse;
                }
            }
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            if (state.linking_node > 0)
            {
                int pin = -1;
                GraphNode* target = NodeByPin(state, canvas_pos, mouse, state.linking_output, pin);
                if (target && target->id != state.linking_node)
                {
                    if (state.linking_output)
                        changed |= AddLink(state, state.linking_node, state.linking_pin, target->id, pin) != 0;
                    else
                        changed |= AddLink(state, target->id, pin, state.linking_node, state.linking_pin) != 0;
                }
                state.linking_node = -1;
                state.linking_pin = -1;
            }
            if (state.box_selecting)
            {
                const ImVec2 a(std::min(state.box_start.x, mouse.x), std::min(state.box_start.y, mouse.y));
                const ImVec2 b(std::max(state.box_start.x, mouse.x), std::max(state.box_start.y, mouse.y));
                for (GraphNode& node : state.nodes)
                {
                    const NodeLayout layout = ComputeLayout(state, node, canvas_pos);
                    if (RectsOverlap(a, b, layout.min, layout.max))
                    {
                        node.selected = true;
                        state.selected_node = node.id;
                    }
                }
                state.box_selecting = false;
            }
            state.dragging_node = -1;
        }

        if (state.dragging_node > 0 && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            GraphNode* node = FindNode(state, state.dragging_node);
            if (node)
            {
                node->position.x += io.MouseDelta.x / state.zoom;
                node->position.y += io.MouseDelta.y / state.zoom;
                if (state.snap_to_grid)
                {
                    node->position.x = std::round(node->position.x / state.grid_size) * state.grid_size;
                    node->position.y = std::round(node->position.y / state.grid_size) * state.grid_size;
                }
                changed = true;
            }
        }

        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !NodeAt(state, canvas_pos, mouse))
        {
            // Double-click empty canvas adds a comment.
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                const ImVec2 graph_pos((mouse.x - canvas_pos.x) / state.zoom - state.pan.x,
                                       (mouse.y - canvas_pos.y) / state.zoom - state.pan.y);
                AddComment(state, "Double-click to edit this comment in your app data.", graph_pos);
                changed = true;
            }
        }

        // Begin box selection once the mouse actually moves.
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && state.dragging_node < 0 && state.linking_node < 0)
        {
            // Only when no node interaction is in progress.
            if (state.box_start.x != 0.0f || state.box_start.y != 0.0f)
            {
                if (std::fabs(mouse.x - state.box_start.x) > 3.0f || std::fabs(mouse.y - state.box_start.y) > 3.0f)
                    state.box_selecting = true;
            }
        }

        // Right click: context menus.
        if (hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Right) &&
            !ImGui::IsMouseDragging(ImGuiMouseButton_Right, 2.0f))
        {
            const ImVec2 graph_pos((mouse.x - canvas_pos.x) / state.zoom - state.pan.x,
                                   (mouse.y - canvas_pos.y) / state.zoom - state.pan.y);
            storage->SetFloat(static_cast<ImGuiID>(context_pos_id), graph_pos.x);
            storage->SetFloat(static_cast<ImGuiID>(context_pos_id + 1), graph_pos.y);
            GraphNode* node = NodeAt(state, canvas_pos, mouse);
            const int link_id = LinkAt(state, canvas_pos, mouse);
            storage->SetInt(context_node_id, node ? node->id : 0);
            storage->SetInt(context_link_id, link_id);
            ImGui::OpenPopup("##graph_context");
        }
    }

    if (ImGui::BeginPopup("##graph_context"))
    {
        const int node_id = storage->GetInt(context_node_id, 0);
        const int link_id = storage->GetInt(context_link_id, 0);
        if (link_id != 0)
        {
            if (ImGui::MenuItem("Delete Link"))
            {
                state.links.erase(std::remove_if(state.links.begin(), state.links.end(),
                                                 [link_id](const GraphLink& link) { return link.id == link_id; }),
                                  state.links.end());
                changed = true;
            }
        }
        else if (node_id != 0)
        {
            if (ImGui::MenuItem("Delete Node"))
            {
                RemoveNode(state, node_id);
                changed = true;
            }
            if (ImGui::MenuItem("Duplicate Node"))
            {
                GraphNode* source = FindNode(state, node_id);
                if (source)
                {
                    GraphNode copy = *source;
                    copy.id = state.next_id++;
                    copy.position.x += 24.0f;
                    copy.position.y += 24.0f;
                    copy.selected = false;
                    state.nodes.push_back(copy);
                    changed = true;
                }
            }
            if (ImGui::MenuItem("Collapse", nullptr, false, true))
            {
                if (GraphNode* node = FindNode(state, node_id))
                {
                    node->collapsed = !node->collapsed;
                    changed = true;
                }
            }
        }
        else
        {
            const ImVec2 graph_pos(storage->GetFloat(context_pos_id, 0.0f), storage->GetFloat(context_pos_id + 1, 0.0f));
            if (ImGui::BeginMenu("Add Node"))
            {
                for (int category = 0; category < static_cast<int>(NodeCategory::Count); ++category)
                {
                    if (ImGui::MenuItem(NodeCategoryName(static_cast<NodeCategory>(category))))
                    {
                        GraphNode& node = AddNode(state, NodeCategoryName(static_cast<NodeCategory>(category)),
                                                  static_cast<NodeCategory>(category), graph_pos);
                        node.inputs.push_back({ "In", "flow", ImVec4(0, 0, 0, 0) });
                        node.outputs.push_back({ "Out", "flow", ImVec4(0, 0, 0, 0) });
                        changed = true;
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Add Comment"))
            {
                AddComment(state, "New comment", graph_pos);
                changed = true;
            }
            if (ImGui::MenuItem("Add Group"))
            {
                AddGroup(state, "Group", graph_pos, ImVec2(320, 220));
                changed = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reset View"))
            {
                state.pan = ImVec2(0, 0);
                state.zoom = 1.0f;
            }
            ImGui::MenuItem("Show Grid", nullptr, &state.show_grid);
            ImGui::MenuItem("Show Minimap", nullptr, &state.show_minimap);
            ImGui::MenuItem("Snap to Grid", nullptr, &state.snap_to_grid);
        }
        ImGui::EndPopup();
    }

    // Dropping assets creates a Data node wired to nothing.
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AMBERUI_ASSET"))
        {
            const char* path = static_cast<const char*>(payload->Data);
            const ImVec2 graph_pos((mouse.x - canvas_pos.x) / state.zoom - state.pan.x,
                                   (mouse.y - canvas_pos.y) / state.zoom - state.pan.y);
            GraphNode& node = AddNode(state, path && path[0] ? path : "Asset", NodeCategory::Data, graph_pos);
            node.inputs.push_back({ "Asset", "asset", ImVec4(0, 0, 0, 0) });
            node.outputs.push_back({ "Ref", "asset", ImVec4(0, 0, 0, 0) });
            changed = true;
        }
        ImGui::EndDragDropTarget();
    }

    if ((hovered || active) && ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        std::vector<int> to_remove;
        for (const GraphNode& node : state.nodes)
        {
            if (node.selected)
                to_remove.push_back(node.id);
        }
        for (const int node_id : to_remove)
            RemoveNode(state, node_id);
        changed |= !to_remove.empty();
    }

    draw->PopClipRect();
    ImGui::PopID();
    return changed;
}

} // namespace AmberUI
