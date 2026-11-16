// Amber UI - Typography implementation
// SPDX-License-Identifier: MIT
#include "AmberUI/Typography.h"

#include <imgui.h>

#include <algorithm>
#include <filesystem>
#include <map>

namespace AmberUI
{
namespace
{
struct TypographyState
{
    ImFont* fonts[static_cast<int>(FontRole::Count)] = { nullptr, nullptr, nullptr, nullptr };
    float scale = 1.0f;
    bool custom = false;
};

std::map<ImGuiContext*, TypographyState>& States()
{
    static std::map<ImGuiContext*, TypographyState> states;
    return states;
}

bool FileExists(const std::string& path)
{
    if (path.empty())
        return false;
    std::error_code error;
    return std::filesystem::is_regular_file(path, error);
}

float RoleSize(const Theme& theme, FontRole role, float scale)
{
    switch (role)
    {
    case FontRole::Mono: return theme.metrics.fontSizeMono * scale;
    case FontRole::Bold: return theme.metrics.fontSizeBase * scale;
    case FontRole::Medium: return theme.metrics.fontSizeBase * scale;
    case FontRole::Regular:
    default: return theme.metrics.fontSizeBase * scale;
    }
}
} // namespace

std::string Typography::FindFontFile(const std::string& explicit_path, const std::string& file_name,
                                     const std::string& assets_dir)
{
    namespace fs = std::filesystem;
    auto try_path = [](const fs::path& path) -> std::string {
        std::error_code error;
        if (fs::is_regular_file(path, error))
            return path.string();
        return std::string();
    };

    if (!explicit_path.empty())
    {
        fs::path path(explicit_path);
        if (path.is_absolute())
        {
            const std::string found = try_path(path);
            if (!found.empty())
                return found;
        }
        if (!assets_dir.empty())
        {
            const std::string found = try_path(fs::path(assets_dir) / explicit_path);
            if (!found.empty())
                return found;
        }
        const std::string found = try_path(path);
        if (!found.empty())
            return found;
        const std::string sub = try_path(fs::path("assets") / explicit_path);
        if (!sub.empty())
            return sub;
        return std::string();
    }

    if (!file_name.empty() && !assets_dir.empty())
    {
        const std::string found = try_path(fs::path(assets_dir) / "fonts" / file_name);
        if (!found.empty())
            return found;
    }
    return std::string();
}

bool Typography::Load(const Theme& theme, ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    if (!context)
        return false;

    ImGuiContext* previous = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(context);
    ImGuiIO& io = ImGui::GetIO();
    TypographyState& state = States()[context];
    state.scale = theme.GetDpiScale();
    state.custom = false;
    for (ImFont*& font : state.fonts)
        font = nullptr;

    if (!io.Fonts)
    {
        ImGui::SetCurrentContext(previous);
        return false;
    }

    std::string assets_dir;
#if defined(AMBERUI_ASSETS_DIR)
    assets_dir = AMBERUI_ASSETS_DIR;
#endif

    const FontRole roles[] = { FontRole::Regular, FontRole::Medium, FontRole::Bold, FontRole::Mono };
    for (const FontRole role : roles)
    {
        const std::string& declared = theme.fonts.Path(role);
        const char* fallback_file = role == FontRole::Mono ? "JetBrainsMono-Regular.ttf" : "Inter-Regular.ttf";
        std::string resolved = FindFontFile(declared, fallback_file, assets_dir);
        if (resolved.empty())
            continue;

        ImFontConfig config;
        config.OversampleH = 2;
        config.OversampleV = 1;
        config.PixelSnapH = false;
        config.FontDataOwnedByAtlas = true;
        const float size = RoleSize(theme, role, state.scale);
        ImFont* font = io.Fonts->AddFontFromFileTTF(resolved.c_str(), size, &config);
        if (font)
        {
            state.fonts[static_cast<int>(role)] = font;
            state.custom = true;
        }
    }

    // Fallbacks: keep every role valid.
    if (state.fonts[static_cast<int>(FontRole::Regular)])
    {
        for (int i = 0; i < static_cast<int>(FontRole::Count); ++i)
        {
            if (!state.fonts[i])
                state.fonts[i] = state.fonts[static_cast<int>(FontRole::Regular)];
        }
    }
    else
    {
        // No font files present: use ImGui's built-in font for every role.
        ImFont* builtin = io.Fonts->Fonts.Size > 0 ? io.Fonts->Fonts[0] : nullptr;
        for (ImFont*& font : state.fonts)
            font = builtin;
    }

    ImGui::SetCurrentContext(previous);
    return state.custom;
}

void Typography::Unload(ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    if (context)
        States().erase(context);
}

ImFont* Typography::Get(FontRole role, ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    const auto it = States().find(context);
    if (it == States().end())
        return nullptr;
    const int index = static_cast<int>(role);
    return index >= 0 && index < static_cast<int>(FontRole::Count) ? it->second.fonts[index] : nullptr;
}

bool Typography::HasCustomFonts(ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    const auto it = States().find(context);
    return it != States().end() && it->second.custom;
}

void Typography::Push(FontRole role, float size)
{
    ImFont* font = Get(role);
    ImGui::PushFont(font, size);
}

void Typography::Pop()
{
    ImGui::PopFont();
}

void Typography::SetScale(float scale, ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    if (context)
        States()[context].scale = std::max(0.5f, scale);
}

float Typography::GetScale(ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    const auto it = States().find(context);
    return it != States().end() ? it->second.scale : 1.0f;
}

} // namespace AmberUI
