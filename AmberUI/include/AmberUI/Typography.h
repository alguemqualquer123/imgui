// Amber UI - Typography
// SPDX-License-Identifier: MIT
#pragma once

#include "AmberUI/ThemeTokens.h"

#include <imgui.h>

#include <string>

namespace AmberUI
{
// Loads the fonts declared by a theme. Font files are optional: when a path is
// empty or the file does not exist, ImGui's current font is kept, so the library
// always works without shipping any font.
//
// Call after CreateContext() and before the first NewFrame(). With backends
// supporting dynamic textures this may also be called later (before NewFrame).
class Typography
{
public:
    static bool Load(const Theme& theme, ImGuiContext* context = nullptr);
    static void Unload(ImGuiContext* context = nullptr);

    static ImFont* Get(FontRole role, ImGuiContext* context = nullptr);
    static bool HasCustomFonts(ImGuiContext* context = nullptr);

    // Push a role with an optional explicit unscaled size (0 = role default).
    static void Push(FontRole role, float size = 0.0f);
    static void Pop();

    static void SetScale(float scale, ImGuiContext* context = nullptr);
    static float GetScale(ImGuiContext* context = nullptr);

    // Resolve `<dir>/<file>` candidates used by the loader.
    static std::string FindFontFile(const std::string& explicit_path, const std::string& file_name, const std::string& assets_dir);
};
} // namespace AmberUI
