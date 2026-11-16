// Amber UI - Theme manager implementation
// SPDX-License-Identifier: MIT
#include "AmberUI/ThemeManager.h"
#include "AmberUI/Theme.h"

#include <imgui.h>

#include <algorithm>
#include <cctype>

namespace AmberUI
{
namespace
{
std::string NormalizeId(const std::string& id)
{
    std::string out;
    out.reserve(id.size());
    for (const char c : id)
    {
        if (std::isalnum(static_cast<unsigned char>(c)))
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}

class ContextScope
{
public:
    explicit ContextScope(ImGuiContext* context)
        : m_previous(ImGui::GetCurrentContext()), m_changed(context && context != m_previous)
    {
        if (m_changed)
            ImGui::SetCurrentContext(context);
    }
    ~ContextScope()
    {
        if (m_changed)
            ImGui::SetCurrentContext(m_previous);
    }

private:
    ImGuiContext* m_previous = nullptr;
    bool m_changed = false;
};
} // namespace

ThemeManager& ThemeManager::Get()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::RegisterBuiltInThemes()
{
    if (m_builtins_registered)
        return;
    m_builtins_registered = true;
    for (const Theme& theme : BuiltInThemes())
        RegisterTheme(theme);
}

void ThemeManager::RegisterTheme(const Theme& theme, const std::string& id)
{
    const std::string key = !id.empty() ? NormalizeId(id) : NormalizeId(theme.name);
    if (key.empty())
        return;
    Theme copy = theme;
    if (copy.name.empty())
        copy.name = ThemeDisplayName(key);
    m_registry[key] = std::move(copy);
}

bool ThemeManager::UnregisterTheme(const std::string& id)
{
    return m_registry.erase(NormalizeId(id)) > 0;
}

bool ThemeManager::ApplyTheme(ImGuiContext* context, const std::string& id)
{
    RegisterBuiltInThemes();
    if (!context)
        context = ImGui::GetCurrentContext();
    if (!context)
        return false;

    const std::string key = NormalizeId(id);
    const auto it = m_registry.find(key);
    if (it == m_registry.end())
        return false;

    ContextState& state = m_contexts[context];
    state.theme = it->second;
    state.has_theme = true;

    ContextScope scope(context);
    ApplyThemeToStyle(ImGui::GetStyle(), state.theme);
    return true;
}

void ThemeManager::SetActiveTheme(ImGuiContext* context, const Theme& theme)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    if (!context)
        return;

    ContextState& state = m_contexts[context];
    state.theme = theme;
    state.has_theme = true;

    ContextScope scope(context);
    ApplyThemeToStyle(ImGui::GetStyle(), state.theme);
}

void ThemeManager::ResetToDefault(ImGuiContext* context)
{
    ApplyTheme(context, kDefaultThemeName);
}

const Theme* ThemeManager::ActiveTheme(ImGuiContext* context) const
{
    if (!context)
        context = ImGui::GetCurrentContext();
    const auto it = m_contexts.find(context);
    if (it == m_contexts.end() || !it->second.has_theme)
        return nullptr;
    return &it->second.theme;
}

const Theme* ThemeManager::GetTheme(const std::string& id) const
{
    const auto it = m_registry.find(NormalizeId(id));
    return it != m_registry.end() ? &it->second : nullptr;
}

std::vector<std::string> ThemeManager::ThemeIds() const
{
    std::vector<std::string> ids;
    ids.reserve(m_registry.size());
    for (const auto& pair : m_registry)
    {
        // Reconstruct a camel-case id from the stored display name.
        std::string id;
        bool capitalize = true;
        for (const char c : pair.second.name)
        {
            if (std::isalnum(static_cast<unsigned char>(c)))
            {
                id += capitalize ? static_cast<char>(std::toupper(static_cast<unsigned char>(c))) : c;
                capitalize = false;
            }
            else
            {
                capitalize = true;
            }
        }
        ids.push_back(id.empty() ? pair.first : id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

void ThemeManager::SetAlpha(float alpha, ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    if (!context)
        return;
    ContextState& state = m_contexts[context];
    if (!state.has_theme)
        ApplyTheme(context, kDefaultThemeName);
    state.theme.alpha = std::clamp(alpha, 0.0f, 1.0f);
    Refresh(context);
}

void ThemeManager::SetDpiScale(float scale, ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    if (!context)
        return;
    ContextState& state = m_contexts[context];
    if (!state.has_theme)
        ApplyTheme(context, kDefaultThemeName);
    state.theme.SetDpiScale(scale);
    Refresh(context);
}

void ThemeManager::Refresh(ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    if (!context)
        return;
    const auto it = m_contexts.find(context);
    if (it == m_contexts.end() || !it->second.has_theme)
        return;
    ContextScope scope(context);
    ApplyThemeToStyle(ImGui::GetStyle(), it->second.theme);
}

void ThemeManager::ReleaseContext(ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    if (context)
        m_contexts.erase(context);
}

} // namespace AmberUI
