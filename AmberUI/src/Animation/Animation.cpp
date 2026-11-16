// Amber UI - Animation helpers
// SPDX-License-Identifier: MIT
#include "AmberUI/Animation.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <cmath>
#include <cfloat>
#include <map>

namespace AmberUI
{
float ApplyEasing(float t, Easing easing)
{
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    switch (easing)
    {
    case Easing::Linear: return t;
    case Easing::EaseIn: return t * t * t;
    case Easing::EaseOut: return 1.0f - std::pow(1.0f - t, 3.0f);
    case Easing::EaseInOut:
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;
    default: return t;
    }
}

namespace Animation
{
namespace
{
std::map<ImGuiContext*, bool>& EnabledMap()
{
    static std::map<ImGuiContext*, bool> map;
    return map;
}

ImGuiID Mix(ImGuiID id, unsigned int salt)
{
    return static_cast<ImGuiID>(id ^ (salt * 2654435761u));
}

float AdvanceTo(const ImGuiID id, float target, float speed, Easing easing)
{
    if (!ImGui::GetCurrentWindowRead())
        return target;
    ImGuiStorage* storage = ImGui::GetStateStorage();
    if (!storage)
        return target;

    constexpr float kUnset = -FLT_MAX;
    const ImGuiID value_key = Mix(id, 0x1u);
    const ImGuiID target_key = Mix(id, 0x2u);
    const ImGuiID start_key = Mix(id, 0x3u);
    const ImGuiID progress_key = Mix(id, 0x4u);

    float value = storage->GetFloat(value_key, kUnset);
    if (value == kUnset)
    {
        storage->SetFloat(value_key, target);
        storage->SetFloat(target_key, target);
        storage->SetFloat(start_key, target);
        storage->SetFloat(progress_key, 1.0f);
        return target;
    }

    float progress = storage->GetFloat(progress_key, 1.0f);
    if (storage->GetFloat(target_key, target) != target)
    {
        storage->SetFloat(start_key, value);
        storage->SetFloat(target_key, target);
        progress = 0.0f;
    }

    if (progress < 1.0f)
    {
        const float dt = ImGui::GetIO().DeltaTime;
        if (dt > 0.0f)
            progress = progress + (1.0f - progress) * (1.0f - std::exp(-speed * dt));
        if (progress > 0.999f)
            progress = 1.0f;
        const float start = storage->GetFloat(start_key, value);
        value = start + (target - start) * ApplyEasing(progress, easing);
        storage->SetFloat(progress_key, progress);
        storage->SetFloat(value_key, value);
    }
    else if (value != target)
    {
        value = target;
        storage->SetFloat(value_key, value);
    }
    return value;
}
} // namespace

void SetEnabled(bool enabled)
{
    EnabledMap()[ImGui::GetCurrentContext()] = enabled;
}

bool IsEnabled()
{
    ImGuiContext* context = ImGui::GetCurrentContext();
    const auto it = EnabledMap().find(context);
    return it == EnabledMap().end() ? true : it->second;
}

void ResetContext(ImGuiContext* context)
{
    if (context)
        EnabledMap().erase(context);
}

float Float(ImGuiID id, float target, float speed, Easing easing)
{
    if (!IsEnabled())
        return target;
    return AdvanceTo(id, target, speed, easing);
}

float Hover(ImGuiID id, bool hovered, float speed, Easing easing)
{
    return Float(id, hovered ? 1.0f : 0.0f, speed, easing);
}

ImVec4 Color(ImGuiID id, const ImVec4& target, float speed, Easing easing)
{
    if (!IsEnabled())
        return target;
    return ImVec4(Float(Mix(id, 0x9E3779B1u), target.x, speed, easing),
                  Float(Mix(id, 0x85EBCA77u), target.y, speed, easing),
                  Float(Mix(id, 0xC2B2AE3Du), target.z, speed, easing),
                  Float(Mix(id, 0x27D4EB2Fu), target.w, speed, easing));
}

float Fade(ImGuiID id, bool visible, float speed)
{
    return Float(id, visible ? 1.0f : 0.0f, speed, Easing::EaseOut);
}

float TimedAlpha(double start_time, float fade_in, float hold, float fade_out)
{
    const double elapsed = ImGui::GetTime() - start_time;
    if (elapsed < 0.0)
        return 0.0f;
    if (elapsed < fade_in)
        return ApplyEasing(static_cast<float>(elapsed / (fade_in > 0.0f ? fade_in : 1.0f)), Easing::EaseOut);
    const double after_hold = elapsed - fade_in;
    if (after_hold < hold)
        return 1.0f;
    const double fade_elapsed = after_hold - hold;
    if (fade_elapsed >= fade_out || fade_out <= 0.0f)
        return 0.0f;
    return 1.0f - ApplyEasing(static_cast<float>(fade_elapsed / fade_out), Easing::EaseIn);
}
} // namespace Animation
} // namespace AmberUI
