// Amber UI - Timeline extension
// SPDX-License-Identifier: MIT
//
// A generic timeline usable by animation, audio, video or sequencing tools.
// It only edits its own visual data (tracks, keyframes, markers, playhead).
#pragma once

#include "AmberUI/ThemeTokens.h"

#include <imgui.h>

#include <string>
#include <vector>

namespace AmberUI
{
struct TimelineKeyframe
{
    double time = 0.0;
    float value = 0.5f; // normalized 0..1, drawn inside the track row
    bool selected = false;
};

struct TimelineTrack
{
    std::string name;
    std::vector<TimelineKeyframe> keyframes;
    bool selected = false;
    bool muted = false;
    bool locked = false;
    ImVec4 color = ImVec4(0, 0, 0, 0); // alpha 0 = automatic accent
};

struct TimelineMarker
{
    double time = 0.0;
    std::string name;
    ImVec4 color = ImVec4(0, 0, 0, 0);
};

struct TimelineState
{
    std::vector<TimelineTrack> tracks;
    std::vector<TimelineMarker> markers;

    double playhead = 0.0;
    double duration = 10.0;
    double view_start = 0.0;
    double view_end = 10.0;
    double selection_start = 0.0;
    double selection_end = 0.0;
    bool selection_active = false;

    bool playing = false;
    bool loop = false;
    bool snapping = true;
    double snap_interval = 0.1;
    float playback_speed = 1.0f;

    int selected_track = -1;
    int selected_keyframe = -1;
    float track_height = 26.0f;
    float label_width = 150.0f;
    bool show_curves = true;
};

TimelineTrack& AddTrack(TimelineState& state, const char* name);
void AddKeyframe(TimelineTrack& track, double time, float value);
TimelineMarker& AddMarker(TimelineState& state, double time, const char* name);
void SortKeyframes(TimelineTrack& track);

// Draw the timeline. Returns true when the data was edited (keyframe moved,
// added/removed, marker added/removed, range changed).
bool Timeline(const char* id, TimelineState& state, const ImVec2& size = ImVec2(0.0f, 0.0f));
} // namespace AmberUI
