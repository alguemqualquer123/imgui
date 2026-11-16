// Amber UI - Node Graph extension
// SPDX-License-Identifier: MIT
//
// Purely visual/interactive graph editing. Amber UI deliberately does not
// execute nodes or model gameplay logic.
#pragma once

#include "AmberUI/ThemeTokens.h"

#include <imgui.h>

#include <string>
#include <vector>

namespace AmberUI
{
enum class NodeCategory
{
    Flow,
    Function,
    Input,
    Output,
    Animation,
    Math,
    Event,
    Data,
    Count
};

const char* NodeCategoryName(NodeCategory category);
ImVec4 NodeCategoryColor(NodeCategory category);

struct GraphPin
{
    std::string name;
    std::string type;      // free-form label, e.g. "float"
    ImVec4 color = ImVec4(0, 0, 0, 0); // alpha 0 = derive from node category
};

struct GraphNode
{
    int id = 0;
    std::string title;
    NodeCategory category = NodeCategory::Flow;
    ImVec2 position = ImVec2(0, 0);
    ImVec2 size = ImVec2(190.0f, 0.0f); // y = 0 -> auto
    bool selected = false;
    bool collapsed = false;
    std::vector<GraphPin> inputs;
    std::vector<GraphPin> outputs;
};

struct GraphLink
{
    int id = 0;
    int from_node = 0;
    int from_pin = 0;
    int to_node = 0;
    int to_pin = 0;
    ImVec4 color = ImVec4(0, 0, 0, 0); // alpha 0 = derive from source category
};

struct GraphGroup
{
    int id = 0;
    std::string title;
    ImVec2 position = ImVec2(0, 0);
    ImVec2 size = ImVec2(300, 200);
    ImVec4 color = ImVec4(0, 0, 0, 0);
    bool selected = false;
};

struct GraphComment
{
    int id = 0;
    std::string text;
    ImVec2 position = ImVec2(0, 0);
    ImVec2 size = ImVec2(220, 90);
    ImVec4 color = ImVec4(0, 0, 0, 0);
};

struct NodeGraphState
{
    std::vector<GraphNode> nodes;
    std::vector<GraphLink> links;
    std::vector<GraphGroup> groups;
    std::vector<GraphComment> comments;

    ImVec2 pan = ImVec2(0, 0);
    float zoom = 1.0f;
    float grid_size = 24.0f;
    bool show_grid = true;
    bool show_minimap = true;
    bool snap_to_grid = false;
    bool allow_multiselect = true;

    // Transient interaction state (owned by the widget).
    int selected_node = -1;
    int dragging_node = -1;
    int linking_node = -1;
    int linking_pin = -1;
    bool linking_output = true;
    bool box_selecting = false;
    ImVec2 box_start = ImVec2(0, 0);
    bool has_drag_payload = false;

    int next_id = 1;
};

// Constructors/utilities.
GraphNode& AddNode(NodeGraphState& state, const char* title, NodeCategory category, const ImVec2& position);
GraphGroup& AddGroup(NodeGraphState& state, const char* title, const ImVec2& position, const ImVec2& size);
GraphComment& AddComment(NodeGraphState& state, const char* text, const ImVec2& position);
int AddLink(NodeGraphState& state, int from_node, int from_pin, int to_node, int to_pin);
void RemoveNode(NodeGraphState& state, int node_id);
void ClearSelection(NodeGraphState& state);
GraphNode* FindNode(NodeGraphState& state, int id);

// Draw the graph. Returns true when the graph content changed (node moved,
// link created/removed, node added, etc.).
bool NodeGraph(const char* id, NodeGraphState& state, const ImVec2& size = ImVec2(0.0f, 0.0f));
} // namespace AmberUI
