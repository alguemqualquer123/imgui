// Amber UI - Widget library
// SPDX-License-Identifier: MIT
//
// All widgets are immediate-mode and preserve Dear ImGui semantics: they use the
// normal ID stack, return true on change/activation, and never own application
// data. They draw with the active theme tokens only.
#pragma once

#include "AmberUI/Icons.h"
#include "AmberUI/ThemeTokens.h"

#include <imgui.h>

#include <functional>
#include <string>
#include <vector>

namespace AmberUI
{
// =============================================================================
// Basic components
// =============================================================================
enum class ButtonVariant
{
    Primary,
    Secondary,
    Ghost,
    Danger
};

bool Button(const char* label, const ImVec2& size = ImVec2(0.0f, 0.0f));
bool ButtonEx(const char* label, ButtonVariant variant, const ImVec2& size = ImVec2(0.0f, 0.0f));
bool PrimaryButton(const char* label, const ImVec2& size = ImVec2(0.0f, 0.0f));
bool SecondaryButton(const char* label, const ImVec2& size = ImVec2(0.0f, 0.0f));
bool GhostButton(const char* label, const ImVec2& size = ImVec2(0.0f, 0.0f));
bool IconButton(Icon icon, const char* tooltip = nullptr, const ImVec2& size = ImVec2(0.0f, 0.0f));

bool ToggleSwitch(const char* label, bool* value, float scale = 1.0f);
bool Checkbox(const char* label, bool* value);
bool RadioButton(const char* label, int* value, int option_value);
bool SearchBar(const char* id, char* buffer, size_t buffer_size, const char* hint = "Search...");
bool TextInput(const char* label, char* buffer, size_t buffer_size, const char* hint = nullptr,
               ImGuiInputTextFlags flags = 0);
bool PasswordInput(const char* label, char* buffer, size_t buffer_size);
bool NumberInput(const char* label, float* value, float step = 1.0f, float min_value = 0.0f,
                 float max_value = 0.0f, const char* format = "%.2f");
bool FloatSlider(const char* label, float* value, float min_value, float max_value,
                 const char* format = "%.2f");
bool RangeSlider(const char* label, float* min_value, float* max_value, float min_limit,
                 float max_limit, const char* format = "%.2f");
bool Dropdown(const char* label, int* current, const char* const items[], int item_count);
bool Dropdown(const char* label, int* current, const std::vector<std::string>& items);

enum class BadgeStyle
{
    Neutral,
    Accent,
    Success,
    Warning,
    Error,
    Info
};
void Badge(const char* text, BadgeStyle style = BadgeStyle::Neutral);
void Tooltip(const char* text);
void LoadingSpinner(const char* id, float radius = 8.0f, float thickness = 2.0f);
void ProgressBar(float fraction, const ImVec2& size = ImVec2(-1.0f, 0.0f), const char* overlay = nullptr,
                 bool animated = true);
bool TabBar(const char* id, const char* const labels[], int count, int* active, bool full_width = false);
bool Breadcrumb(const char* id, const char* const labels[], int count, int* selected);
void Divider(const char* label = nullptr);
void SectionHeader(const char* label, const char* right_text = nullptr);

// =============================================================================
// Property grid / inspector controls
// =============================================================================
bool BeginPropertyGrid(const char* id, float label_width_ratio = 0.42f, ImGuiTableFlags extra_flags = 0);
void EndPropertyGrid();

bool PropertySection(const char* label, bool default_open = true);
bool PropertyGroup(const char* label);
void EndPropertyGroup();

bool PropertyRowFloat(const char* label, float* value, float speed = 0.1f, float min_value = 0.0f,
                      float max_value = 0.0f, const char* format = "%.3f");
bool PropertyRowInt(const char* label, int* value, int min_value = 0, int max_value = 0);
bool PropertyRowBool(const char* label, bool* value);
bool PropertyRowText(const char* label, char* buffer, size_t buffer_size);
bool PropertyRowColor(const char* label, ImVec4* color, bool with_alpha = true);
bool PropertyRowCombo(const char* label, int* current, const char* const items[], int item_count);
bool PropertyRowCustom(const char* label, const std::function<bool()>& widget);

bool Vector2Control(const char* label, float values[2], float speed = 0.05f, const char* format = "%.3f");
bool Vector3Control(const char* label, float values[3], float speed = 0.05f, const char* format = "%.3f");
bool Vector4Control(const char* label, float values[4], float speed = 0.05f, const char* format = "%.3f");
bool TransformControl(const char* label, float position[3], float rotation[3], float scale[3]);
bool ColorField(const char* label, ImVec4* color, bool with_alpha = true);
bool RangeControl(const char* label, float* min_value, float* max_value, float low_limit = 0.0f,
                  float high_limit = 1.0f);

struct GradientStop
{
    float position = 0.0f; // 0..1
    ImVec4 color = ImVec4(1, 1, 1, 1);
};

struct Gradient
{
    std::vector<GradientStop> stops;
    ImVec4 Evaluate(float t) const;
    void Sort();
    static Gradient Default();
};

struct Curve
{
    // Points are normalized: x and y in [0,1]. X is kept sorted.
    std::vector<ImVec2> points;
    float Evaluate(float x) const;
    static Curve Default();
};

bool GradientEditor(const char* id, Gradient& gradient, float height = 28.0f);
bool CurveEditor(const char* id, Curve& curve, const ImVec2& size = ImVec2(0.0f, 160.0f));

// =============================================================================
// Tree view
// =============================================================================
struct TreeItem
{
    std::string label;
    Icon icon = Icon::File;
    bool expanded = false;
    bool selected = false;
    bool is_leaf = false;
    void* user_data = nullptr;
    std::vector<TreeItem> children;
};

struct TreeViewOptions
{
    bool show_icons = true;
    bool indent_guides = true;
    bool border = false;
};

// Returns true when the selection changed. Selection state is stored on the
// items (selected flags are mutually exclusive).
bool TreeView(const char* id, std::vector<TreeItem>& items, const ImVec2& size = ImVec2(0.0f, 0.0f),
              const TreeViewOptions& options = TreeViewOptions());

// =============================================================================
// File / asset browsers
// =============================================================================
struct FileEntry
{
    std::string name;
    std::string path;
    std::string extension;
    bool is_directory = false;
    unsigned long long size = 0;
};

struct FileBrowserState
{
    std::string directory;
    std::string selected;
    std::string error;
    bool show_hidden = false;
    bool folders_only = false;
    float thumbnail_size = 64.0f;
    int view_mode = 0; // 0 = grid, 1 = list
    std::vector<std::string> history;
    int history_index = -1;
    // Internal cache (refreshed lazily; safe to ignore).
    std::vector<FileEntry> entries;
    std::string cached_directory;
    bool cache_valid = false;
    bool force_refresh = true;
};

// Real filesystem browser. Returns true when the selection changed; a double
// click on a directory navigates into it.
bool FileBrowser(const char* id, FileBrowserState& state, const ImVec2& size = ImVec2(0.0f, 0.0f));

struct AssetItem
{
    std::string name;
    std::string type;        // "Material", "Mesh", "Texture", ...
    std::string path;
    ImTextureID thumbnail = 0;
    ImVec4 tint = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
};

struct AssetBrowserState
{
    char search[128] = "";
    int selected = -1;
    bool selecting = false;
    float thumbnail_size = 76.0f;
};

bool AssetBrowser(const char* id, std::vector<AssetItem>& assets, AssetBrowserState& state,
                  const ImVec2& size = ImVec2(0.0f, 0.0f));
bool AssetCard(const char* id, const AssetItem& asset, bool selected, const ImVec2& size);

using ThumbnailDrawCallback = void (*)(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max, int index,
                                       void* user_data);
bool ThumbnailGrid(const char* id, int item_count, const ImVec2& item_size, ThumbnailDrawCallback callback,
                   void* user_data, int* selected_index, const ImVec2& size = ImVec2(0.0f, 0.0f));

// =============================================================================
// Shell components
// =============================================================================
bool BeginToolbar(const char* id, float height = 0.0f);
void EndToolbar();
void ToolbarSeparator();

bool BeginStatusBar(const char* id, float height = 0.0f);
void EndStatusBar();
void StatusBarText(const char* text);
void StatusBarTextFaded(const char* text);
void StatusBarRight(const char* text);

// =============================================================================
// Notifications
// =============================================================================
enum class ToastLevel
{
    Info,
    Success,
    Warning,
    Error
};

class NotificationCenter
{
public:
    static NotificationCenter& Get();

    void Push(const char* title, const char* message = nullptr, ToastLevel level = ToastLevel::Info,
              float duration = 4.5f);
    void Clear();
    size_t Count() const { return m_items.size(); }

    // Draws the toast stack as an overlay in the current viewport.
    void Draw();

private:
    struct Item
    {
        std::string title;
        std::string message;
        ToastLevel level = ToastLevel::Info;
        double start_time = 0.0;
        float duration = 4.5f;
        float alpha = 0.0f;
        bool closing = false;
    };
    std::vector<Item> m_items;
};

inline void Notify(const char* title, const char* message = nullptr, ToastLevel level = ToastLevel::Info,
                   float duration = 4.5f)
{
    NotificationCenter::Get().Push(title, message, level, duration);
}

// =============================================================================
// Command palette
// =============================================================================
struct PaletteCommand
{
    std::string name;
    std::string category;
    std::string shortcut;
};

class CommandPalette
{
public:
    static CommandPalette& Get();

    void SetCommands(std::vector<PaletteCommand> commands);
    void Open();
    void Close();
    bool IsOpen() const { return m_open; }

    // Submit the palette. Returns the index of an executed command, or -1.
    int Draw();

private:
    std::vector<PaletteCommand> m_commands;
    bool m_open = false;
    bool m_focus_input = false;
    char m_filter[128] = "";
    int m_selected = 0;
    int m_scroll_to = -1;
};

// =============================================================================
// Console / log viewer
// =============================================================================
struct LogEntry
{
    std::string text;
    ToastLevel level = ToastLevel::Info;
    double timestamp = 0.0;
};

struct ConsoleState
{
    bool auto_scroll = true;
    bool show_info = true;
    bool show_success = true;
    bool show_warning = true;
    bool show_error = true;
    bool show_timestamps = true;
    char input[512] = "";
    std::vector<LogEntry> entries;
    int history_index = -1;
    std::vector<std::string> history;

    void Add(const char* text, ToastLevel level = ToastLevel::Info);
    void Clear() { entries.clear(); }
};

// `on_command` is invoked when the user submits the input line.
bool Console(const char* id, ConsoleState& state, const ImVec2& size = ImVec2(0.0f, 0.0f),
             const std::function<void(const char*)>& on_command = nullptr);

struct LogViewerState
{
    char filter[128] = "";
    bool auto_scroll = true;
    bool show_info = true;
    bool show_warning = true;
    bool show_error = true;
};

void LogViewer(const char* id, const std::vector<LogEntry>& entries, LogViewerState& state,
               const ImVec2& size = ImVec2(0.0f, 0.0f));

// =============================================================================
// Inspector
// =============================================================================
bool BeginInspector(const char* id, const ImVec2& size = ImVec2(0.0f, 0.0f));
void EndInspector();
// Collapsible component block that also opens a property grid.
bool InspectorComponent(const char* label, Icon icon = Icon::Cube, bool default_open = true);

// =============================================================================
// Utilities
// =============================================================================
ImVec4 TokenColor(ColorToken token);
ImU32 TokenU32(ColorToken token, float alpha_multiplier = 1.0f);
float MetricScale();
float Metric(float value);

// Set the next item width to a fraction of the available content width.
void SetNextItemWidthFraction(float fraction);
// Draw text using the secondary text token.
void TextSecondary(const char* text);
void TextDisabled(const char* text);
void TextWrappedMuted(const char* text);
} // namespace AmberUI
