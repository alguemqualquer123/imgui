// Amber UI - Theme style application and presets
// SPDX-License-Identifier: MIT
#pragma once

#include "AmberUI/ThemeTokens.h"

#include <imgui.h>

#include <string>
#include <vector>

namespace AmberUI
{
// Apply every color and metric of `theme` to an ImGuiStyle. This touches all
// ImGuiCol entries supported by the ImGui version being compiled against.
void ApplyThemeToStyle(ImGuiStyle& style, const Theme& theme);

// Built-in presets. Each returns a fully populated Theme.
Theme AmberDark();
Theme AmberGraphite();
Theme AmberMidnight();
Theme AmberOLED();
Theme AmberLight();
Theme AmberHighContrast();
Theme UE5Dark();

// All built-in presets, in registry order.
std::vector<Theme> BuiltInThemes();

// Name of the default preset used by Initialize().
constexpr const char* kDefaultThemeName = "AmberDark";

// Convert an internal preset id ("AmberDark") to a display name ("Amber Dark").
std::string ThemeDisplayName(const std::string& theme_id);

} // namespace AmberUI
