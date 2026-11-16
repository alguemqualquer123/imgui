// Amber UI - Public lifecycle API
// SPDX-License-Identifier: MIT
#include "AmberUI/AmberUI.h"

#include "AmberUI/ThemeManager.h"
#include "AmberUI/Typography.h"

#include <imgui.h>

namespace AmberUI
{
void Initialize(ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    if (!context)
        return;

    ThemeManager::Get().RegisterBuiltInThemes();
    ThemeManager::Get().ApplyTheme(context, kDefaultThemeName);
}

void Shutdown(ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    if (!context)
        return;
    ThemeManager::Get().ResetToDefault(context);
    ImGuiContext* previous = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(context);
    Animation::SetEnabled(false);
    ImGui::SetCurrentContext(previous);
    ThemeManager::Get().ReleaseContext(context);
}

void ShutdownContext(ImGuiContext* context)
{
    if (!context)
        return;
    Animation::ResetContext(context);
    Typography::Unload(context);
    ThemeManager::Get().ReleaseContext(context);
}

bool ApplyTheme(const char* theme_id, ImGuiContext* context)
{
    if (!theme_id)
        return false;
    return ThemeManager::Get().ApplyTheme(context, theme_id);
}

const Theme* CurrentTheme(ImGuiContext* context)
{
    return ThemeManager::Get().ActiveTheme(context);
}

void RegisterTheme(const Theme& theme)
{
    ThemeManager::Get().RegisterTheme(theme);
}

bool LoadThemeFonts(ImGuiContext* context)
{
    if (!context)
        context = ImGui::GetCurrentContext();
    const Theme* theme = ThemeManager::Get().ActiveTheme(context);
    if (!theme)
        return false;
    return Typography::Load(*theme, context);
}

void SetAnimationsEnabled(bool enabled)
{
    Animation::SetEnabled(enabled);
}

} // namespace AmberUI
