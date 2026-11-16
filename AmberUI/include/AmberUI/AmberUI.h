// Amber UI - A professional Dear ImGui theme and widget library
// SPDX-License-Identifier: MIT
//
// Quick start:
//   ImGui::CreateContext();
//   AmberUI::Initialize();
//   AmberUI::ApplyTheme("AmberDark");
//   ...
//   AmberUI::PrimaryButton("Save");
//   ...
//   AmberUI::Shutdown();
//
// Amber UI never creates/destroys the ImGui context, never owns the platform or
// renderer backend, and never calls NewFrame()/Render(). The host application
// stays in charge of its frame loop.
#pragma once

#include "AmberUI/Animation.h"
#include "AmberUI/Icons.h"
#include "AmberUI/Serialization.h"
#include "AmberUI/Theme.h"
#include "AmberUI/ThemeManager.h"
#include "AmberUI/ThemeTokens.h"
#include "AmberUI/Typography.h"
#include "AmberUI/Widgets.h"

// Optional extensions are header-guarded so the core library has no dependency
// on them.
#if defined(AMBERUI_INCLUDE_EXTENSIONS)
#include "AmberUI/NodeGraph.h"
#include "AmberUI/Timeline.h"
#endif

#define AMBERUI_VERSION_MAJOR 1
#define AMBERUI_VERSION_MINOR 0
#define AMBERUI_VERSION_PATCH 0
#define AMBERUI_VERSION_STRING "1.0.0"

namespace AmberUI
{
// Prepare Amber UI for the given context (nullptr = current). Registers the
// built-in themes and applies AmberDark. Safe to call once per context.
void Initialize(ImGuiContext* context = nullptr);

// Apply the default theme again and disable animations.
void Shutdown(ImGuiContext* context = nullptr);

// Release all Amber UI state associated with a context. Call this before
// ImGui::DestroyContext(). Does not touch style or fonts.
void ShutdownContext(ImGuiContext* context);

// Convenience wrapper around ThemeManager::ApplyTheme.
bool ApplyTheme(const char* theme_id, ImGuiContext* context = nullptr);

// The active theme, or nullptr if Amber UI was not initialized for the context.
const Theme* CurrentTheme(ImGuiContext* context = nullptr);

// Register a custom theme (JSON import helper lives in Serialization.h).
void RegisterTheme(const Theme& theme);

// Fonts declared by the active theme. Call after Initialize() and before the
// first frame. Returns false when no font file could be loaded (built-in font
// stays in use, which is perfectly valid).
bool LoadThemeFonts(ImGuiContext* context = nullptr);

// Enable/disable widget animations globally (per context).
void SetAnimationsEnabled(bool enabled);
} // namespace AmberUI
