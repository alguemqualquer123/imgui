// Amber UI - Theme manager
// SPDX-License-Identifier: MIT
#pragma once

#include "AmberUI/Theme.h"
#include "AmberUI/ThemeTokens.h"

#include <imgui.h>

#include <map>
#include <string>
#include <vector>

namespace AmberUI
{
// Per-ImGui-context theme state. Amber UI never shares theme state between
// contexts: the active theme, registry and DPI scale are keyed by ImGuiContext*.
class ThemeManager
{
public:
    static ThemeManager& Get();

    // Install the built-in presets (idempotent, cheap).
    void RegisterBuiltInThemes();

    // Register/overwrite a custom theme. `theme.name` is the registry key.
    void RegisterTheme(const Theme& theme, const std::string& id = std::string());
    bool UnregisterTheme(const std::string& id);

    // Apply a registered theme to `context` (nullptr = current context).
    bool ApplyTheme(ImGuiContext* context, const std::string& id);
    bool ApplyTheme(const std::string& id) { return ApplyTheme(ImGui::GetCurrentContext(), id); }

    // Replace the active theme with an already-built object (e.g. live editor).
    void SetActiveTheme(ImGuiContext* context, const Theme& theme);
    void SetActiveTheme(const Theme& theme) { SetActiveTheme(ImGui::GetCurrentContext(), theme); }

    // Restore the default preset for this context.
    void ResetToDefault(ImGuiContext* context = nullptr);

    const Theme* ActiveTheme(ImGuiContext* context) const;
    const Theme* ActiveTheme() const { return ActiveTheme(ImGui::GetCurrentContext()); }

    // Registry access. `GetTheme` returns nullptr when unknown.
    const Theme* GetTheme(const std::string& id) const;
    std::vector<std::string> ThemeIds() const;

    // Adjust the active theme in place and reapply it.
    void SetAlpha(float alpha, ImGuiContext* context = nullptr);
    void SetDpiScale(float scale, ImGuiContext* context = nullptr);
    void Refresh(ImGuiContext* context = nullptr);

    // Free state tied to a destroyed context. Called by AmberUI::ShutdownContext
    // (also safe to call for any context before ImGui::DestroyContext).
    void ReleaseContext(ImGuiContext* context);

    // Number of contexts currently tracked (useful for tests/leak checks).
    size_t TrackedContextCount() const { return m_contexts.size(); }

private:
    struct ContextState
    {
        Theme theme;
        bool has_theme = false;
    };

    ThemeManager() = default;

    std::map<std::string, Theme> m_registry;
    std::map<ImGuiContext*, ContextState> m_contexts;
    bool m_builtins_registered = false;
};

} // namespace AmberUI
