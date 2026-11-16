// Amber UI - Built-in monochrome vector icons
// SPDX-License-Identifier: MIT
//
// Amber UI ships no icon font: all icons are drawn as vector primitives through
// ImDrawList. This keeps the library free of third-party asset licensing, crisp
// at any DPI, and independent from font merging.
#pragma once

#include <imgui.h>

namespace AmberUI
{
enum class Icon
{
    None,
    Folder,
    FolderOpen,
    File,
    Image,
    Code,
    Settings,
    Search,
    Close,
    Plus,
    Minus,
    Check,
    ChevronDown,
    ChevronRight,
    ArrowLeft,
    ArrowRight,
    Play,
    Pause,
    Stop,
    SkipBack,
    SkipForward,
    Save,
    Open,
    Undo,
    Redo,
    Grid,
    Layers,
    Cube,
    Light,
    Camera,
    Material,
    Audio,
    Video,
    Link,
    Node,
    Timeline,
    Console,
    Warning,
    Error,
    Info,
    Success,
    Eye,
    Lock,
    Trash,
    Copy,
    Filter,
    Sun,
    Moon,
    Grip,
    Dot,
    Count
};

// Draw an icon centered in [center - size/2, center + size/2].
void DrawIcon(ImDrawList* draw_list, Icon icon, const ImVec2& center, float size, ImU32 color, float thickness = 1.5f);

// Draw an icon aligned to a line of text at the current cursor and advance the
// cursor like an image would.
void IconImage(Icon icon, float size, ImU32 color);

const char* IconName(Icon icon);
} // namespace AmberUI
