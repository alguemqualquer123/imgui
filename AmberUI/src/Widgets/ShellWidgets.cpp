// Amber UI - Toolbar, status bar, notifications, command palette, console, inspector
// SPDX-License-Identifier: MIT
#include "AmberUI/Widgets.h"

#include "AmberUI/Animation.h"
#include "AmberUI/ThemeManager.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

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

ColorToken LevelToken(ToastLevel level)
{
    switch (level)
    {
    case ToastLevel::Success: return ColorToken::Success;
    case ToastLevel::Warning: return ColorToken::Warning;
    case ToastLevel::Error: return ColorToken::Error;
    case ToastLevel::Info:
    default: return ColorToken::Info;
    }
}

struct ToolbarFrame
{
    ImVec2 start_pos;
    float height = 0.0f;
};

std::vector<ToolbarFrame>& ToolbarStack()
{
    static std::vector<ToolbarFrame> stack;
    return stack;
}

bool Contains(const std::string& haystack, const char* needle)
{
    if (!needle || !needle[0])
        return true;
    const size_t length = std::strlen(needle);
    for (size_t i = 0; i + length <= haystack.size(); ++i)
    {
        bool equal = true;
        for (size_t j = 0; j < length; ++j)
        {
            if (std::tolower(static_cast<unsigned char>(haystack[i + j])) !=
                std::tolower(static_cast<unsigned char>(needle[j])))
            {
                equal = false;
                break;
            }
        }
        if (equal)
            return true;
    }
    return false;
}
} // namespace

// =============================================================================
// Toolbar
// =============================================================================
bool BeginToolbar(const char* id, float height)
{
    const float s = S();
    const float bar_height = height > 0.0f ? height * s
                                           : (ThemeForWidgets() ? ThemeForWidgets()->metrics.toolbarHeight * s : 36.0f * s);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;

    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + bar_height), U32(ColorToken::PanelBackground));
    draw->AddLine(ImVec2(pos.x, pos.y + bar_height - 1.0f), ImVec2(pos.x + width, pos.y + bar_height - 1.0f),
                  U32(ColorToken::Border));
    ToolbarStack().push_back({ pos, bar_height });

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * s, 3.0f * s));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f * s, 0.0f));
    ImGui::BeginChild(id, ImVec2(width, bar_height), ImGuiChildFlags_None,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPosY(std::max(2.0f, (bar_height - ImGui::GetFrameHeight()) * 0.5f - 2.0f * s));
    return true;
}

void EndToolbar()
{
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    if (ToolbarStack().empty())
        return;
    const ToolbarFrame frame = ToolbarStack().back();
    ToolbarStack().pop_back();
    ImGui::SetCursorScreenPos(ImVec2(frame.start_pos.x, frame.start_pos.y + frame.height + ImGui::GetStyle().ItemSpacing.y));
}

void ToolbarSeparator()
{
    const float s = S();
    const float height = ImGui::GetFrameHeight();
    ImGui::SameLine(0.0f, 6.0f * s);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(8.0f * s, height));
    ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x + 4.0f * s, pos.y), ImVec2(pos.x + 4.0f * s, pos.y + height),
                                        U32(ColorToken::Border));
    ImGui::SameLine(0.0f, 6.0f * s);
}

// =============================================================================
// Status bar
// =============================================================================
bool BeginStatusBar(const char* id, float height)
{
    const float s = S();
    const float bar_height = height > 0.0f ? height * s
                                           : (ThemeForWidgets() ? ThemeForWidgets()->metrics.statusBarHeight * s : 24.0f * s);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + bar_height), U32(ColorToken::PanelBackground));
    ImGui::GetWindowDrawList()->AddLine(pos, ImVec2(pos.x + width, pos.y), U32(ColorToken::Border));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * s, 2.0f * s));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f * s, 0.0f));
    ImGui::PushID(id);
    ImGui::BeginChild("##status_bar", ImVec2(width, bar_height), ImGuiChildFlags_None,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPosY(std::max(1.0f, (bar_height - ImGui::GetTextLineHeight()) * 0.5f));
    return true;
}

void EndStatusBar()
{
    ImGui::EndChild();
    ImGui::PopID();
    ImGui::PopStyleVar(2);
}

void StatusBarText(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, TokenColorSafe(ColorToken::TextSecondary));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

void StatusBarTextFaded(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, TokenColorSafe(ColorToken::TextMuted));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

void StatusBarRight(const char* text)
{
    const float width = ImGui::CalcTextSize(text).x;
    const float available = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x * 2.0f;
    ImGui::SameLine(std::max(available - width, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, TokenColorSafe(ColorToken::TextMuted));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

// =============================================================================
// Notifications
// =============================================================================
NotificationCenter& NotificationCenter::Get()
{
    static NotificationCenter instance;
    return instance;
}

void NotificationCenter::Push(const char* title, const char* message, ToastLevel level, float duration)
{
    Item item;
    item.title = title ? title : "";
    item.message = message ? message : "";
    item.level = level;
    item.start_time = ImGui::GetTime();
    item.duration = duration > 0.0f ? duration : 4.5f;
    m_items.push_back(std::move(item));
}

void NotificationCenter::Clear()
{
    m_items.clear();
}

void NotificationCenter::Draw()
{
    if (m_items.empty())
        return;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (!viewport)
        return;

    const float s = S();
    const float toast_width = 320.0f * s;
    const float spacing = 6.0f * s;
    const ImVec2 work_pos = viewport->WorkPos;
    const ImVec2 work_size = viewport->WorkSize;

    for (size_t i = 0; i < m_items.size();)
    {
        Item& item = m_items[i];
        const float alpha = Animation::TimedAlpha(item.start_time, 0.18f, item.duration, 0.35f);
        if (alpha <= 0.0f && ImGui::GetTime() > item.start_time + 0.5)
        {
            m_items.erase(m_items.begin() + static_cast<std::ptrdiff_t>(i));
            continue;
        }
        ++i;
    }
    if (m_items.empty())
        return;

    ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - toast_width - 16.0f * s, work_pos.y + 16.0f * s));
    ImGui::SetNextWindowSize(ImVec2(toast_width, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, spacing));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin("##amber_toasts", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_AlwaysAutoResize);

    for (size_t i = 0; i < m_items.size();)
    {
        Item& item = m_items[i];
        const float alpha = Animation::TimedAlpha(item.start_time, 0.18f, item.duration, 0.35f);
        if (alpha <= 0.01f)
        {
            ++i;
            continue;
        }
        const float height = item.message.empty() ? 42.0f * s : 58.0f * s;
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::PushID(static_cast<int>(i));
        const bool clicked = ImGui::InvisibleButton("##toast", ImVec2(toast_width, height));
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImVec2 min = pos;
        const ImVec2 max(pos.x + toast_width, pos.y + height);
        const ImVec4 bg = ThemeForWidgets() ? ThemeForWidgets()->Color(ColorToken::SecondaryBackground) : ImVec4(0.15f, 0.15f, 0.15f, 1);
        draw->AddRectFilled(min, max, ImGui::GetColorU32(WithAlpha(bg, 0.96f * alpha)), 4.0f * s);
        draw->AddRect(min, max, U32(ColorToken::BorderStrong, 0.6f * alpha), 4.0f * s);
        const ImU32 accent = U32(LevelToken(item.level), alpha);
        draw->AddRectFilled(min, ImVec2(min.x + 3.0f * s, max.y), accent, 2.0f * s);

        const ImVec2 title_size = ImGui::CalcTextSize(item.title.c_str());
        draw->AddText(ImVec2(min.x + 14.0f * s, min.y + 10.0f * s), U32(ColorToken::TextPrimary, alpha),
                      item.title.c_str());
        if (!item.message.empty())
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::SetCursorScreenPos(ImVec2(min.x + 14.0f * s, min.y + 10.0f * s + title_size.y + 4.0f * s));
            ImGui::PushStyleColor(ImGuiCol_Text, WithAlpha(TokenColorSafe(ColorToken::TextSecondary), alpha));
            ImGui::PushTextWrapPos(max.x - 30.0f * s);
            ImGui::TextUnformatted(item.message.c_str());
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }

        const ImVec2 close_center(max.x - 14.0f * s, min.y + 14.0f * s);
        draw->AddLine(ImVec2(close_center.x - 3.0f * s, close_center.y - 3.0f * s),
                      ImVec2(close_center.x + 3.0f * s, close_center.y + 3.0f * s),
                      U32(ColorToken::TextMuted, alpha), 1.3f);
        draw->AddLine(ImVec2(close_center.x + 3.0f * s, close_center.y - 3.0f * s),
                      ImVec2(close_center.x - 3.0f * s, close_center.y + 3.0f * s),
                      U32(ColorToken::TextMuted, alpha), 1.3f);
        if (clicked && ImGui::GetIO().MousePos.x > max.x - 28.0f * s && ImGui::GetIO().MousePos.y < min.y + 28.0f * s)
        {
            m_items.erase(m_items.begin() + static_cast<std::ptrdiff_t>(i));
            ImGui::PopID();
            continue;
        }
        ImGui::PopID();
        ++i;
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

// =============================================================================
// Command palette
// =============================================================================
CommandPalette& CommandPalette::Get()
{
    static CommandPalette instance;
    return instance;
}

void CommandPalette::SetCommands(std::vector<PaletteCommand> commands)
{
    m_commands = std::move(commands);
}

void CommandPalette::Open()
{
    m_open = true;
    m_focus_input = true;
    m_filter[0] = '\0';
    m_selected = 0;
    m_scroll_to = -1;
}

void CommandPalette::Close()
{
    m_open = false;
}

int CommandPalette::Draw()
{
    if (!m_open)
        return -1;

    const float s = S();
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 center(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f, viewport->WorkPos.y + viewport->WorkSize.y * 0.18f);
    ImGui::GetBackgroundDrawList()->AddRectFilled(viewport->WorkPos,
                                                  ImVec2(viewport->WorkPos.x + viewport->WorkSize.x,
                                                         viewport->WorkPos.y + viewport->WorkSize.y),
                                                  IM_COL32(0, 0, 0, 110));

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(560.0f * s, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f * s, 10.0f * s));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f * s);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, TokenColorSafe(ColorToken::SecondaryBackground));
    ImGui::PushStyleColor(ImGuiCol_Border, TokenColorSafe(ColorToken::BorderStrong));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
    int executed = -1;
    if (ImGui::Begin("##amber_command_palette", nullptr, flags))
    {
        if (m_focus_input)
        {
            ImGui::SetKeyboardFocusHere();
            m_focus_input = false;
        }
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f * s, 7.0f * s));
        const bool filter_changed = ImGui::InputTextWithHint("##palette_filter", "Type a command...", m_filter,
                                                             sizeof(m_filter));
        ImGui::PopStyleVar();
        if (filter_changed)
            m_selected = 0;

        std::vector<int> matches;
        for (int i = 0; i < static_cast<int>(m_commands.size()); ++i)
        {
            const PaletteCommand& command = m_commands[static_cast<size_t>(i)];
            if (Contains(command.name, m_filter) || Contains(command.category, m_filter))
                matches.push_back(i);
        }
        if (matches.empty())
            m_selected = 0;

        const bool move_down = ImGui::IsKeyPressed(ImGuiKey_DownArrow);
        const bool move_up = ImGui::IsKeyPressed(ImGuiKey_UpArrow);
        if (move_down && !matches.empty())
            m_selected = (m_selected + 1) % static_cast<int>(matches.size());
        if (move_up && !matches.empty())
            m_selected = (m_selected - 1 + static_cast<int>(matches.size())) % static_cast<int>(matches.size());

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        if (ImGui::BeginChild("##results", ImVec2(0.0f, std::min(320.0f * s, ImGui::GetTextLineHeightWithSpacing() * 12.0f)),
                              ImGuiChildFlags_None))
        {
            for (int row = 0; row < static_cast<int>(matches.size()); ++row)
            {
                const PaletteCommand& command = m_commands[static_cast<size_t>(matches[static_cast<size_t>(row)])];
                const bool selected = row == m_selected;
                ImGui::PushID(row);
                const ImVec2 pos = ImGui::GetCursorScreenPos();
                const float height = ImGui::GetFrameHeight() + 6.0f * s;
                if (ImGui::InvisibleButton("##command", ImVec2(ImGui::GetContentRegionAvail().x, height)))
                {
                    executed = matches[static_cast<size_t>(row)];
                    m_open = false;
                }
                const ImVec2 max(pos.x + ImGui::GetItemRectSize().x, pos.y + height);
                ImDrawList* draw = ImGui::GetWindowDrawList();
                if (selected)
                    draw->AddRectFilled(pos, max, U32(ColorToken::Accent, 0.18f), 3.0f * s);
                else if (ImGui::IsItemHovered())
                    draw->AddRectFilled(pos, max, U32(ColorToken::FrameHovered, 0.5f), 3.0f * s);

                const ImVec2 text_size = ImGui::CalcTextSize(command.name.c_str());
                draw->AddText(ImVec2(pos.x + 10.0f * s, pos.y + (height - text_size.y) * 0.5f),
                              U32(selected ? ColorToken::TextPrimary : ColorToken::TextSecondary),
                              command.name.c_str());
                const float category_width = ImGui::CalcTextSize(command.category.c_str()).x;
                draw->AddText(ImVec2(max.x - category_width - 10.0f * s, pos.y + (height - text_size.y) * 0.5f),
                              U32(ColorToken::TextDisabled), command.category.c_str());
                if (selected)
                    ImGui::SetScrollHereY(0.5f);
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();

        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
        {
            if (!matches.empty())
            {
                executed = matches[static_cast<size_t>(m_selected)];
                m_open = false;
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            m_open = false;
    }
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
    return executed;
}

// =============================================================================
// Console & log viewer
// =============================================================================
void ConsoleState::Add(const char* text, ToastLevel level)
{
    LogEntry entry;
    entry.text = text ? text : "";
    entry.level = level;
    entry.timestamp = ImGui::GetTime();
    entries.push_back(std::move(entry));
}

bool Console(const char* id, ConsoleState& state, const ImVec2& size,
             const std::function<void(const char*)>& on_command)
{
    ImGui::PushID(id);
    const float s = S();
    bool submitted = false;

    if (ImGui::Button("Clear"))
    {
        state.Clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &state.auto_scroll);
    ImGui::SameLine();
    ImGui::Checkbox("Info", &state.show_info);
    ImGui::SameLine();
    ImGui::Checkbox("Warn", &state.show_warning);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &state.show_error);
    ImGui::SameLine();
    ImGui::Checkbox("Time", &state.show_timestamps);

    const ImGuiWindowFlags child_flags = ImGuiWindowFlags_HorizontalScrollbar;
    if (ImGui::BeginChild("##console_output", size, ImGuiChildFlags_Borders, child_flags))
    {
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(state.entries.size()));
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                const LogEntry& entry = state.entries[static_cast<size_t>(i)];
                if ((entry.level == ToastLevel::Info && !state.show_info) ||
                    (entry.level == ToastLevel::Success && !state.show_success) ||
                    (entry.level == ToastLevel::Warning && !state.show_warning) ||
                    (entry.level == ToastLevel::Error && !state.show_error))
                    continue;
                if (state.show_timestamps)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, TokenColorSafe(ColorToken::TextDisabled));
                    ImGui::Text("[%.2f]", entry.timestamp);
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                }
                ImGui::PushStyleColor(ImGuiCol_Text, TokenColorSafe(LevelToken(entry.level)));
                ImGui::TextUnformatted(entry.text.c_str());
                ImGui::PopStyleColor();
            }
        }
        if (state.auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f)
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f * s, 5.0f * s));
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive())
        ImGui::SetKeyboardFocusHere();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputTextWithHint("##console_input", "Enter a command...", state.input, sizeof(state.input),
                                 ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if (state.input[0])
        {
            state.Add(state.input, ToastLevel::Info);
            state.history.push_back(state.input);
            state.history_index = -1;
            if (on_command)
                on_command(state.input);
            state.input[0] = '\0';
            submitted = true;
        }
        ImGui::SetKeyboardFocusHere(-1);
    }
    ImGui::PopStyleVar();
    ImGui::PopID();
    return submitted;
}

void LogViewer(const char* id, const std::vector<LogEntry>& entries, LogViewerState& state, const ImVec2& size)
{
    ImGui::PushID(id);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##log_filter", "Filter...", state.filter, sizeof(state.filter));
    if (ImGui::BeginChild("##log_entries", size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(entries.size()));
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                const LogEntry& entry = entries[static_cast<size_t>(i)];
                if ((entry.level == ToastLevel::Info && !state.show_info) ||
                    (entry.level == ToastLevel::Warning && !state.show_warning) ||
                    (entry.level == ToastLevel::Error && !state.show_error))
                    continue;
                if (!Contains(entry.text, state.filter))
                    continue;
                ImGui::PushStyleColor(ImGuiCol_Text, TokenColorSafe(ColorToken::TextDisabled));
                ImGui::Text("[%.2f]", entry.timestamp);
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, TokenColorSafe(LevelToken(entry.level)));
                ImGui::TextUnformatted(entry.text.c_str());
                ImGui::PopStyleColor();
            }
        }
        if (state.auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f)
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::PopID();
}

// =============================================================================
// Inspector
// =============================================================================
bool BeginInspector(const char* id, const ImVec2& size)
{
    ImGui::PushID(id);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * S(), 8.0f * S()));
    return ImGui::BeginChild("##inspector", size, ImGuiChildFlags_Borders,
                             ImGuiWindowFlags_AlwaysVerticalScrollbar);
}

void EndInspector()
{
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopID();
}

bool InspectorComponent(const char* label, Icon icon, bool default_open)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed;
    if (default_open)
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    ImGui::PushID(label);
    const bool open = ImGui::CollapsingHeader(label, flags);
    const ImVec2 rect_max = ImGui::GetItemRectMax();
    const ImVec2 rect_min = ImGui::GetItemRectMin();
    DrawIcon(ImGui::GetWindowDrawList(), icon,
             ImVec2(rect_max.x - 12.0f * S(), (rect_min.y + rect_max.y) * 0.5f), 12.0f * S(),
             U32(open ? ColorToken::Accent : ColorToken::TextMuted), 1.3f);
    ImGui::PopID();
    if (open)
    {
        char grid_id[160];
        std::snprintf(grid_id, sizeof(grid_id), "##component_grid_%s", label ? label : "");
        BeginPropertyGrid(grid_id, 0.45f);
    }
    return open;
}

} // namespace AmberUI
