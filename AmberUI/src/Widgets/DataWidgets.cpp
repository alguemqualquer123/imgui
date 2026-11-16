// Amber UI - Tree view, file browser and asset browser
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
#include <filesystem>
#include <string>
#include <utility>
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

Icon IconForExtension(const std::string& extension)
{
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".tga" ||
        extension == ".bmp" || extension == ".hdr" || extension == ".exr")
        return Icon::Image;
    if (extension == ".cpp" || extension == ".h" || extension == ".hpp" || extension == ".c" ||
        extension == ".cs" || extension == ".py" || extension == ".lua" || extension == ".json")
        return Icon::Code;
    if (extension == ".wav" || extension == ".mp3" || extension == ".ogg" || extension == ".flac")
        return Icon::Audio;
    if (extension == ".mp4" || extension == ".mov" || extension == ".webm" || extension == ".avi")
        return Icon::Video;
    if (extension == ".obj" || extension == ".fbx" || extension == ".gltf" || extension == ".glb")
        return Icon::Cube;
    return Icon::File;
}

std::string LowerExtension(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension;
}

std::string FormatSize(unsigned long long size)
{
    char buffer[32];
    if (size < 1024)
        std::snprintf(buffer, sizeof(buffer), "%llu B", size);
    else if (size < 1024ull * 1024ull)
        std::snprintf(buffer, sizeof(buffer), "%.1f KB", static_cast<double>(size) / 1024.0);
    else if (size < 1024ull * 1024ull * 1024ull)
        std::snprintf(buffer, sizeof(buffer), "%.1f MB", static_cast<double>(size) / (1024.0 * 1024.0));
    else
        std::snprintf(buffer, sizeof(buffer), "%.2f GB", static_cast<double>(size) / (1024.0 * 1024.0 * 1024.0));
    return std::string(buffer);
}

bool ContainsCaseInsensitive(const std::string& haystack, const char* needle)
{
    if (!needle || !needle[0])
        return true;
    const size_t needle_length = std::strlen(needle);
    if (needle_length > haystack.size())
        return false;
    for (size_t i = 0; i + needle_length <= haystack.size(); ++i)
    {
        bool match = true;
        for (size_t j = 0; j < needle_length; ++j)
        {
            if (std::tolower(static_cast<unsigned char>(haystack[i + j])) !=
                std::tolower(static_cast<unsigned char>(needle[j])))
            {
                match = false;
                break;
            }
        }
        if (match)
            return true;
    }
    return false;
}

void ClearSelection(std::vector<TreeItem>& items)
{
    for (TreeItem& item : items)
    {
        item.selected = false;
        ClearSelection(item.children);
    }
}

void FlattenTree(std::vector<TreeItem>& items, int depth, std::vector<std::pair<TreeItem*, int>>& out)
{
    for (TreeItem& item : items)
    {
        out.emplace_back(&item, depth);
        if (item.expanded && !item.children.empty())
            FlattenTree(item.children, depth + 1, out);
    }
}

void RefreshDirectory(FileBrowserState& state)
{
    namespace fs = std::filesystem;
    state.entries.clear();
    state.error.clear();
    if (state.directory.empty())
        state.directory = fs::current_path().string();
    std::error_code error;
    if (!fs::is_directory(state.directory, error))
    {
        state.directory = fs::current_path().string();
        state.error = "Directory not found; using working directory.";
    }
    fs::directory_iterator iterator(state.directory, fs::directory_options::skip_permission_denied, error);
    if (error)
    {
        state.error = "Cannot read directory: " + error.message();
        state.cache_valid = true;
        state.cached_directory = state.directory;
        return;
    }
    for (const fs::directory_entry& entry : iterator)
    {
        std::error_code entry_error;
        const bool is_directory = entry.is_directory(entry_error);
        const std::string name = entry.path().filename().string();
        if (name == "." || name == "..")
            continue;
        if (!state.show_hidden && !name.empty() && name[0] == '.')
            continue;
        if (state.folders_only && !is_directory)
            continue;
        FileEntry file;
        file.name = name;
        file.path = entry.path().string();
        file.is_directory = is_directory;
        file.extension = is_directory ? std::string() : LowerExtension(entry.path());
        if (!is_directory)
        {
            const auto size = entry.file_size(entry_error);
            file.size = entry_error ? 0ull : static_cast<unsigned long long>(size);
        }
        state.entries.push_back(std::move(file));
    }
    std::sort(state.entries.begin(), state.entries.end(), [](const FileEntry& a, const FileEntry& b) {
        if (a.is_directory != b.is_directory)
            return a.is_directory > b.is_directory;
        return a.name < b.name;
    });
    state.cached_directory = state.directory;
    state.cache_valid = true;
}
} // namespace

// =============================================================================
// Tree view
// =============================================================================
bool TreeView(const char* id, std::vector<TreeItem>& items, const ImVec2& size, const TreeViewOptions& options)
{
    ImGui::PushID(id);
    ImGuiChildFlags child_flags = ImGuiChildFlags_None;
    if (options.border)
        child_flags |= ImGuiChildFlags_Borders;
    const bool open = ImGui::BeginChild("##tree", size, child_flags, ImGuiWindowFlags_HorizontalScrollbar);
    bool selection_changed = false;
    if (open)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f * S()));
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 12.0f * S());
        std::vector<std::pair<TreeItem*, int>> visible;
        FlattenTree(items, 0, visible);

        const float row_height = std::max(ImGui::GetTextLineHeight() + 4.0f * S(), 20.0f * S());
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(visible.size()), row_height);
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                TreeItem* item = visible[static_cast<size_t>(i)].first;
                const int depth = visible[static_cast<size_t>(i)].second;
                ImGui::PushID(item);
                const float indent = depth * ImGui::GetStyle().IndentSpacing;
                const float width = std::max(ImGui::GetContentRegionAvail().x, 80.0f);

                const bool clicked = ImGui::InvisibleButton("##row", ImVec2(width, row_height),
                                                            ImGuiButtonFlags_MouseButtonLeft);
                const bool hovered = ImGui::IsItemHovered();
                const ImVec2 min = ImGui::GetItemRectMin();
                const ImVec2 max = ImGui::GetItemRectMax();
                ImDrawList* draw = ImGui::GetWindowDrawList();

                if (item->selected)
                    draw->AddRectFilled(min, max, U32(ColorToken::Accent, 0.20f), 2.0f * S());
                else if (hovered)
                    draw->AddRectFilled(min, max, U32(ColorToken::FrameHovered, 0.65f), 2.0f * S());

                float text_x = min.x + indent + 4.0f * S();
                const bool has_children = !item->children.empty() && !item->is_leaf;
                if (has_children)
                {
                    const ImVec2 arrow_center(min.x + indent + 8.0f * S(), (min.y + max.y) * 0.5f);
                    DrawIcon(draw, item->expanded ? Icon::ChevronDown : Icon::ChevronRight, arrow_center, 10.0f * S(),
                             U32(ColorToken::TextMuted), 1.3f);
                    if (clicked && ImGui::GetIO().MousePos.x < min.x + indent + 18.0f * S())
                    {
                        item->expanded = !item->expanded;
                    }
                    text_x += 16.0f * S();
                }
                else
                {
                    text_x += 16.0f * S();
                }

                if (options.show_icons)
                {
                    const ImVec2 icon_center(text_x + 7.0f * S(), (min.y + max.y) * 0.5f);
                    DrawIcon(draw, item->icon, icon_center, 13.0f * S(),
                             U32(item->selected ? ColorToken::Accent : ColorToken::TextMuted), 1.3f);
                    text_x += 20.0f * S();
                }

                const ImVec4 text_color = TokenColorSafe(item->selected ? ColorToken::TextPrimary : ColorToken::TextSecondary);
                draw->AddText(ImVec2(text_x, min.y + (row_height - ImGui::GetTextLineHeight()) * 0.5f),
                              ImGui::GetColorU32(text_color), item->label.c_str());

                if (options.indent_guides && depth > 0)
                {
                    for (int guide = 0; guide < depth; ++guide)
                    {
                        const float x = min.x - indent + (guide + 1) * ImGui::GetStyle().IndentSpacing - 2.0f;
                        draw->AddLine(ImVec2(x, min.y), ImVec2(x, max.y), U32(ColorToken::Border, 0.5f));
                    }
                }

                if (clicked && !(has_children && ImGui::GetIO().MousePos.x < min.x + indent + 18.0f * S()))
                {
                    ClearSelection(items);
                    item->selected = true;
                    selection_changed = true;
                }
                if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && has_children)
                    item->expanded = !item->expanded;

                ImGui::PopID();
            }
        }
        ImGui::PopStyleVar(2);
    }
    ImGui::EndChild();
    ImGui::PopID();
    return selection_changed;
}

// =============================================================================
// File browser
// =============================================================================
bool FileBrowser(const char* id, FileBrowserState& state, const ImVec2& size)
{
    namespace fs = std::filesystem;
    ImGui::PushID(id);
    bool selection_changed = false;
    const float s = S();

    if (state.directory.empty())
        state.directory = fs::current_path().string();
    if (!state.cache_valid || state.cached_directory != state.directory || state.force_refresh)
    {
        RefreshDirectory(state);
        state.force_refresh = false;
    }

    if (!state.history.empty() && state.history_index >= 0 &&
        state.history_index < static_cast<int>(state.history.size()) &&
        state.history[static_cast<size_t>(state.history_index)] != state.directory)
    {
        state.history[static_cast<size_t>(state.history_index)] = state.directory;
    }

    // Toolbar
    if (ImGui::Button("<", ImVec2(0.0f, 0.0f)))
    {
        if (state.history_index > 0)
        {
            --state.history_index;
            state.directory = state.history[static_cast<size_t>(state.history_index)];
            state.force_refresh = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("^"))
    {
        const fs::path parent = fs::path(state.directory).parent_path();
        if (!parent.empty() && parent != state.directory)
            state.directory = parent.string();
        state.force_refresh = true;
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(state.directory.c_str());
    ImGui::SameLine();
    const float toggle_width = 72.0f * s;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - toggle_width - ImGui::GetStyle().WindowPadding.x);
    if (ImGui::Button(state.view_mode == 0 ? "Grid" : "List", ImVec2(toggle_width, 0.0f)))
        state.view_mode = state.view_mode == 0 ? 1 : 0;
    ImGui::SameLine();
    if (ImGui::Button("R"))
        state.force_refresh = true;

    if (!state.error.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, TokenColorSafe(ColorToken::Error));
        ImGui::TextUnformatted(state.error.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    const ImVec2 child_size(size.x, size.y > 0.0f ? size.y - ImGui::GetCursorPosY() : 0.0f);
    if (ImGui::BeginChild("##files", child_size, ImGuiChildFlags_None))
    {
        auto select_entry = [&](const FileEntry& entry) {
            if (state.selected != entry.path)
            {
                state.selected = entry.path;
                selection_changed = true;
            }
        };
        auto open_entry = [&](const FileEntry& entry) {
            if (entry.is_directory)
            {
                if (state.history_index + 1 < static_cast<int>(state.history.size()))
                    state.history.erase(state.history.begin() + state.history_index + 1, state.history.end());
                state.history.push_back(entry.path);
                state.history_index = static_cast<int>(state.history.size()) - 1;
                state.directory = entry.path;
                state.selected.clear();
                state.force_refresh = true;
            }
        };

        if (state.view_mode == 1)
        {
            const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
            if (ImGui::BeginTable("##list", 3, flags))
            {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 90.0f * s);
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80.0f * s);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();
                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(state.entries.size()));
                while (clipper.Step())
                {
                    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                    {
                        const FileEntry& entry = state.entries[static_cast<size_t>(i)];
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushID(i);
                        const bool selected = state.selected == entry.path;
                        if (ImGui::Selectable(entry.name.c_str(), selected,
                                              ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
                        {
                            select_entry(entry);
                            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                                open_entry(entry);
                        }
                        ImGui::PopID();
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(entry.is_directory ? "Folder" : entry.extension.c_str());
                        ImGui::TableSetColumnIndex(2);
                        if (!entry.is_directory)
                            ImGui::TextUnformatted(FormatSize(entry.size).c_str());
                    }
                }
                ImGui::EndTable();
            }
        }
        else
        {
            const float thumbnail = std::max(28.0f, state.thumbnail_size * s);
            const float cell_width = thumbnail + 24.0f * s;
            const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / cell_width));
            const int rows = (static_cast<int>(state.entries.size()) + columns - 1) / columns;
            ImGuiListClipper clipper;
            clipper.Begin(rows, (thumbnail + 34.0f * s));
            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
                {
                    for (int column = 0; column < columns; ++column)
                    {
                        const int index = row * columns + column;
                        if (index >= static_cast<int>(state.entries.size()))
                            break;
                        const FileEntry& entry = state.entries[static_cast<size_t>(index)];
                        if (column > 0)
                            ImGui::SameLine(column * cell_width);
                        ImGui::PushID(index);
                        const ImVec2 cell_pos = ImGui::GetCursorScreenPos();
                        const bool clicked = ImGui::InvisibleButton("##cell", ImVec2(cell_width - 6.0f * s, thumbnail + 30.0f * s),
                                                                    ImGuiButtonFlags_MouseButtonLeft);
                        const bool hovered = ImGui::IsItemHovered();
                        const bool selected = state.selected == entry.path;
                        ImDrawList* draw = ImGui::GetWindowDrawList();
                        const ImVec2 cell_min = ImGui::GetItemRectMin();
                        const ImVec2 cell_max = ImGui::GetItemRectMax();
                        if (selected)
                            draw->AddRectFilled(cell_min, cell_max, U32(ColorToken::Accent, 0.18f), 3.0f * s);
                        else if (hovered)
                            draw->AddRectFilled(cell_min, cell_max, U32(ColorToken::FrameHovered, 0.55f), 3.0f * s);

                        const ImVec2 thumb_min(cell_pos.x + (cell_width - thumbnail) * 0.5f, cell_pos.y + 4.0f * s);
                        const ImVec2 thumb_max(thumb_min.x + thumbnail, thumb_min.y + thumbnail);
                        const ImVec4 tint = entry.is_directory ? TokenColorSafe(ColorToken::Accent) : TokenColorSafe(ColorToken::TextMuted);
                        draw->AddRectFilled(thumb_min, thumb_max, U32(ColorToken::SecondaryBackground), 3.0f * s);
                        draw->AddRect(thumb_min, thumb_max, U32(ColorToken::Border), 3.0f * s);
                        DrawIcon(draw, entry.is_directory ? Icon::Folder : IconForExtension(entry.extension),
                                 ImVec2((thumb_min.x + thumb_max.x) * 0.5f, (thumb_min.y + thumb_max.y) * 0.5f),
                                 thumbnail * 0.42f, ImGui::GetColorU32(tint), 1.4f);

                        char name_buffer[48];
                        std::snprintf(name_buffer, sizeof(name_buffer), "%s", entry.name.c_str());
                        if (ImGui::CalcTextSize(name_buffer).x > cell_width - 6.0f * s)
                        {
                            size_t length = std::strlen(name_buffer);
                            while (length > 4 && ImGui::CalcTextSize(name_buffer).x > cell_width - 12.0f * s)
                            {
                                name_buffer[length - 1] = '\0';
                                name_buffer[length - 2] = '.';
                                name_buffer[length - 3] = '.';
                                --length;
                            }
                        }
                        draw->AddText(ImVec2(cell_min.x + 4.0f * s, thumb_max.y + 4.0f * s),
                                      U32(selected ? ColorToken::TextPrimary : ColorToken::TextSecondary), name_buffer);

                        if (clicked)
                        {
                            select_entry(entry);
                            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                                open_entry(entry);
                        }
                        ImGui::PopID();
                    }
                }
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopID();
    return selection_changed;
}

// =============================================================================
// Asset browser
// =============================================================================
bool AssetCard(const char* id, const AssetItem& asset, bool selected, const ImVec2& size)
{
    ImGui::PushID(id);
    const float s = S();
    const ImVec2 card_size(size.x > 0.0f ? size.x : 104.0f * s, size.y > 0.0f ? size.y : 108.0f * s);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const bool clicked = ImGui::InvisibleButton("##asset_card", card_size);
    const bool hovered = ImGui::IsItemHovered();
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload("AMBERUI_ASSET", asset.path.c_str(), asset.path.size() + 1);
        ImGui::TextUnformatted(asset.name.c_str());
        ImGui::EndDragDropSource();
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 min = pos;
    const ImVec2 max(pos.x + card_size.x, pos.y + card_size.y);
    const bool hot = hovered || selected;
    draw->AddRectFilled(min, max, U32(selected ? ColorToken::PanelBackground : ColorToken::SecondaryBackground, 0.9f),
                        3.0f * s);
    draw->AddRect(min, max, U32(hot ? ColorToken::Accent : ColorToken::Border, hot ? 0.65f : 1.0f), 3.0f * s, 0,
                  hot ? 1.5f : 1.0f);

    const float padding = 8.0f * s;
    const ImVec2 thumb_min(min.x + padding, min.y + padding);
    const ImVec2 thumb_max(max.x - padding, max.y - 28.0f * s);
    if (asset.thumbnail)
    {
        draw->AddImage(asset.thumbnail, thumb_min, thumb_max);
    }
    else
    {
        draw->AddRectFilled(thumb_min, thumb_max, U32(ColorToken::Background), 2.0f * s);
        const Icon icon = asset.type == "Material" ? Icon::Material
                          : asset.type == "Mesh" ? Icon::Cube
                          : asset.type == "Texture" ? Icon::Image
                          : asset.type == "Audio" ? Icon::Audio
                          : asset.type == "Script" ? Icon::Code
                                                   : Icon::File;
        DrawIcon(draw, icon, ImVec2((thumb_min.x + thumb_max.x) * 0.5f, (thumb_min.y + thumb_max.y) * 0.5f),
                 std::min(thumb_max.x - thumb_min.x, thumb_max.y - thumb_min.y) * 0.32f,
                 ImGui::GetColorU32(asset.tint), 1.5f);
    }

    const ImVec2 text_size = ImGui::CalcTextSize(asset.name.c_str());
    draw->AddText(ImVec2(min.x + padding, max.y - 24.0f * s),
                  U32(selected ? ColorToken::TextPrimary : ColorToken::TextSecondary),
                  text_size.x > card_size.x - padding * 2.0f ? asset.type.c_str() : asset.name.c_str());
    draw->AddText(ImVec2(min.x + padding, max.y - 12.0f * s), U32(ColorToken::TextDisabled), asset.type.c_str());

    ImGui::PopID();
    return clicked;
}

bool AssetBrowser(const char* id, std::vector<AssetItem>& assets, AssetBrowserState& state, const ImVec2& size)
{
    ImGui::PushID(id);
    const float s = S();
    bool changed = false;

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##asset_search", "Filter assets...", state.search, sizeof(state.search));
    ImGui::Spacing();

    if (ImGui::BeginChild("##assets", size, ImGuiChildFlags_None))
    {
        const float card_width = std::max(64.0f, state.thumbnail_size * s + 28.0f);
        const float card_height = state.thumbnail_size * s + 40.0f;
        const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / card_width));
        std::vector<int> visible;
        for (int i = 0; i < static_cast<int>(assets.size()); ++i)
        {
            if (ContainsCaseInsensitive(assets[static_cast<size_t>(i)].name, state.search))
                visible.push_back(i);
        }
        const int rows = (static_cast<int>(visible.size()) + columns - 1) / columns;
        ImGuiListClipper clipper;
        clipper.Begin(rows, card_height + 6.0f * s);
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                for (int column = 0; column < columns; ++column)
                {
                    const int visible_index = row * columns + column;
                    if (visible_index >= static_cast<int>(visible.size()))
                        break;
                    const int asset_index = visible[static_cast<size_t>(visible_index)];
                    if (column > 0)
                        ImGui::SameLine(column * card_width);
                    ImGui::PushID(asset_index);
                    if (AssetCard("##card", assets[static_cast<size_t>(asset_index)], state.selected == asset_index,
                                  ImVec2(card_width - 6.0f, card_height)))
                    {
                        state.selected = asset_index;
                        changed = true;
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        state.selected = asset_index;
                        changed = true;
                    }
                    ImGui::PopID();
                }
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopID();
    return changed;
}

bool ThumbnailGrid(const char* id, int item_count, const ImVec2& item_size, ThumbnailDrawCallback callback,
                   void* user_data, int* selected_index, const ImVec2& size)
{
    ImGui::PushID(id);
    bool changed = false;
    if (ImGui::BeginChild("##grid", size, ImGuiChildFlags_None))
    {
        const float cell_width = item_size.x + 6.0f * S();
        const float cell_height = item_size.y + 6.0f * S();
        const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / cell_width));
        const int rows = (item_count + columns - 1) / columns;
        ImGuiListClipper clipper;
        clipper.Begin(rows, cell_height);
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                for (int column = 0; column < columns; ++column)
                {
                    const int index = row * columns + column;
                    if (index >= item_count)
                        break;
                    if (column > 0)
                        ImGui::SameLine(column * cell_width);
                    ImGui::PushID(index);
                    const ImVec2 pos = ImGui::GetCursorScreenPos();
                    const bool clicked = ImGui::InvisibleButton("##thumb", item_size);
                    const bool hovered = ImGui::IsItemHovered();
                    const bool selected = selected_index && *selected_index == index;
                    ImDrawList* draw = ImGui::GetWindowDrawList();
                    const ImVec2 min = pos;
                    const ImVec2 max(pos.x + item_size.x, pos.y + item_size.y);
                    draw->AddRectFilled(min, max, U32(ColorToken::Background), 3.0f * S());
                    if (callback)
                        callback(draw, min, max, index, user_data);
                    draw->AddRect(min, max,
                                  U32(selected ? ColorToken::Accent : (hovered ? ColorToken::BorderStrong : ColorToken::Border)),
                                  3.0f * S(), 0, selected ? 1.6f : 1.0f);
                    if (clicked && selected_index)
                    {
                        *selected_index = index;
                        changed = true;
                    }
                    ImGui::PopID();
                }
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopID();
    return changed;
}

} // namespace AmberUI
