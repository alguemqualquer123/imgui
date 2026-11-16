// Amber UI - Animation helpers
// SPDX-License-Identifier: MIT
#pragma once

#include <imgui.h>

namespace AmberUI
{
enum class Easing
{
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut
};

float ApplyEasing(float t, Easing easing);

namespace Animation
{
// Global enable flag. When false, every helper returns its target immediately
// and no animation state is stored. Per-context: stored in the ImGui context.
void SetEnabled(bool enabled);
bool IsEnabled();
// Forget the enabled flag for a destroyed context (called by ShutdownContext).
void ResetContext(ImGuiContext* context);

// Frame-rate independent smoothing (uses ImGui::GetIO().DeltaTime).
// The state lives in the current window's ImGuiStorage keyed by `id`, so it is
// released automatically with the window: no unbounded global animation map.
float Float(ImGuiID id, float target, float speed = 14.0f, Easing easing = Easing::EaseOut);
float Hover(ImGuiID id, bool hovered, float speed = 16.0f, Easing easing = Easing::EaseOut);
ImVec4 Color(ImGuiID id, const ImVec4& target, float speed = 14.0f, Easing easing = Easing::EaseOut);
float Fade(ImGuiID id, bool visible, float speed = 12.0f);

// Immediate mode helper used for tooltips/notifications: returns a 0..1 alpha
// driven by the elapsed time since `start_time`.
float TimedAlpha(double start_time, float fade_in, float hold, float fade_out);
} // namespace Animation
} // namespace AmberUI
