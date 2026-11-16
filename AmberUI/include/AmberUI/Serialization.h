// Amber UI - Theme serialization (JSON + C++ export)
// SPDX-License-Identifier: MIT
#pragma once

#include "AmberUI/ThemeTokens.h"

#include <string>

namespace AmberUI
{
// JSON format (stable, human editable). Versioned with "formatVersion": 1.
std::string ThemeToJson(const Theme& theme, bool pretty = true);
bool ThemeFromJson(const char* json, size_t size, Theme& out_theme, std::string* error = nullptr);
bool ThemeFromJson(const std::string& json, Theme& out_theme, std::string* error = nullptr);

bool SaveThemeToFile(const Theme& theme, const std::string& path, std::string* error = nullptr);
bool LoadThemeFromFile(const std::string& path, Theme& out_theme, std::string* error = nullptr);

// Emit a ready-to-compile C++ function returning the theme.
std::string ThemeToCpp(const Theme& theme, const std::string& function_name = "MyTheme");

// Register a theme directly from a JSON string. Returns false + error on parse
// failure; a successfully parsed theme is added to the ThemeManager registry.
bool ImportThemeFromJson(const std::string& json, std::string* error = nullptr);

} // namespace AmberUI
