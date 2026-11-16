// Amber UI - Theme serialization
// SPDX-License-Identifier: MIT
#include "AmberUI/Serialization.h"
#include "AmberUI/ThemeManager.h"

#include "Json.h"

#include <cstdio>
#include <fstream>
#include <sstream>

namespace AmberUI
{
namespace
{
Json::Value ToJson(const Theme& theme)
{
    Json::Value root = Json::MakeObject();
    Json::Set(root, "formatVersion", Json::MakeNumber(1));
    Json::Set(root, "name", Json::MakeString(theme.name));
    Json::Set(root, "alpha", Json::MakeNumber(theme.alpha));
    Json::Set(root, "dpiScale", Json::MakeNumber(theme.dpiScale));

    Json::Value colors = Json::MakeObject();
    for (int i = 0; i < kColorTokenCount; ++i)
        Json::Set(colors, ColorTokenName(static_cast<ColorToken>(i)),
                  Json::MakeString(ColorToHex(theme.colors[static_cast<size_t>(i)])));
    Json::Set(root, "colors", std::move(colors));

    Json::Value metrics = Json::MakeObject();
    auto put_float = [&metrics](const char* key, float value) { Json::Set(metrics, key, Json::MakeNumber(value)); };
    auto put_vec2 = [&metrics](const char* key, const ImVec2& value) {
        Json::Value array = Json::MakeArray();
        array.array.push_back(Json::MakeNumber(value.x));
        array.array.push_back(Json::MakeNumber(value.y));
        Json::Set(metrics, key, std::move(array));
    };

    put_vec2("windowPadding", theme.metrics.windowPadding);
    put_vec2("framePadding", theme.metrics.framePadding);
    put_vec2("itemSpacing", theme.metrics.itemSpacing);
    put_vec2("itemInnerSpacing", theme.metrics.itemInnerSpacing);
    put_vec2("cellPadding", theme.metrics.cellPadding);
    put_vec2("windowTitleAlign", theme.metrics.windowTitleAlign);
    put_vec2("buttonTextAlign", theme.metrics.buttonTextAlign);
    put_vec2("selectableTextAlign", theme.metrics.selectableTextAlign);

    put_float("windowRounding", theme.metrics.windowRounding);
    put_float("childRounding", theme.metrics.childRounding);
    put_float("frameRounding", theme.metrics.frameRounding);
    put_float("popupRounding", theme.metrics.popupRounding);
    put_float("scrollbarRounding", theme.metrics.scrollbarRounding);
    put_float("grabRounding", theme.metrics.grabRounding);
    put_float("tabRounding", theme.metrics.tabRounding);
    put_float("windowBorderSize", theme.metrics.windowBorderSize);
    put_float("childBorderSize", theme.metrics.childBorderSize);
    put_float("frameBorderSize", theme.metrics.frameBorderSize);
    put_float("popupBorderSize", theme.metrics.popupBorderSize);
    put_float("tabBorderSize", theme.metrics.tabBorderSize);
    put_float("scrollbarSize", theme.metrics.scrollbarSize);
    put_float("grabMinSize", theme.metrics.grabMinSize);
    put_float("indentSpacing", theme.metrics.indentSpacing);
    put_float("separatorTextBorder", theme.metrics.separatorTextBorder);
    put_float("windowMinWidth", theme.metrics.windowMinWidth);
    put_float("windowMinHeight", theme.metrics.windowMinHeight);
    put_float("fontSizeBase", theme.metrics.fontSizeBase);
    put_float("fontSizeSmall", theme.metrics.fontSizeSmall);
    put_float("fontSizeTitle", theme.metrics.fontSizeTitle);
    put_float("fontSizeMono", theme.metrics.fontSizeMono);
    put_float("rowHeight", theme.metrics.rowHeight);
    put_float("toolbarHeight", theme.metrics.toolbarHeight);
    put_float("statusBarHeight", theme.metrics.statusBarHeight);
    put_float("controlHeight", theme.metrics.controlHeight);
    Json::Set(root, "metrics", std::move(metrics));

    Json::Value fonts = Json::MakeObject();
    Json::Set(fonts, "regular", Json::MakeString(theme.fonts.regular));
    Json::Set(fonts, "medium", Json::MakeString(theme.fonts.medium));
    Json::Set(fonts, "bold", Json::MakeString(theme.fonts.bold));
    Json::Set(fonts, "mono", Json::MakeString(theme.fonts.mono));
    Json::Set(root, "fonts", std::move(fonts));
    return root;
}

bool ColorFromJson(const Json::Value& value, ImVec4& out)
{
    if (value.IsString())
    {
        out = HexToColor(value.string.c_str());
        return true;
    }
    if (value.IsArray() && (value.array.size() == 3 || value.array.size() == 4))
    {
        out.x = static_cast<float>(value.array[0].NumberOr(1.0));
        out.y = static_cast<float>(value.array[1].NumberOr(1.0));
        out.z = static_cast<float>(value.array[2].NumberOr(1.0));
        out.w = value.array.size() == 4 ? static_cast<float>(value.array[3].NumberOr(1.0)) : 1.0f;
        return true;
    }
    return false;
}

void ReadColor(const Json::Value* colors, ColorToken token, ImVec4& target)
{
    if (!colors)
        return;
    const Json::Value* value = colors->Find(ColorTokenName(token));
    if (!value)
        return;
    ImVec4 parsed;
    if (ColorFromJson(*value, parsed))
        target = parsed;
}

std::string SanitizeIdentifier(const std::string& name)
{
    std::string out;
    bool capitalize = true;
    for (const char c : name)
    {
        if (std::isalnum(static_cast<unsigned char>(c)))
        {
            out += capitalize ? static_cast<char>(std::toupper(static_cast<unsigned char>(c))) : c;
            capitalize = false;
        }
        else
        {
            capitalize = true;
        }
    }
    if (out.empty() || std::isdigit(static_cast<unsigned char>(out[0])))
        out = "Theme" + out;
    return out;
}
} // namespace

std::string ThemeToJson(const Theme& theme, bool pretty)
{
    return Json::Serialize(ToJson(theme), pretty);
}

bool ThemeFromJson(const char* json, size_t size, Theme& out_theme, std::string* error)
{
    if (!json)
    {
        if (error) *error = "null JSON input";
        return false;
    }
    Json::Value root;
    std::string parse_error;
    if (!Json::Parse(json, size, root, parse_error))
    {
        if (error) *error = parse_error;
        return false;
    }
    if (!root.IsObject())
    {
        if (error) *error = "theme JSON root must be an object";
        return false;
    }

    Theme theme;
    if (const Json::Value* name = root.Find("name"); name && name->IsString())
        theme.name = name->string;
    if (const Json::Value* alpha = root.Find("alpha"))
        theme.alpha = static_cast<float>(alpha->NumberOr(theme.alpha));
    if (const Json::Value* dpi = root.Find("dpiScale"))
        theme.dpiScale = static_cast<float>(dpi->NumberOr(theme.dpiScale));

    if (const Json::Value* colors = root.Find("colors"); colors && colors->IsObject())
    {
        for (int i = 0; i < kColorTokenCount; ++i)
            ReadColor(colors, static_cast<ColorToken>(i), theme.colors[static_cast<size_t>(i)]);
    }

    if (const Json::Value* metrics = root.Find("metrics"); metrics && metrics->IsObject())
    {
        auto read_float = [metrics](const char* key, float& field) {
            if (const Json::Value* value = metrics->Find(key); value && value->IsNumber())
                field = static_cast<float>(value->number);
        };
        auto read_vec2 = [metrics](const char* key, ImVec2& field) {
            if (const Json::Value* value = metrics->Find(key); value && value->IsArray() && value->array.size() >= 2)
            {
                field.x = static_cast<float>(value->array[0].NumberOr(field.x));
                field.y = static_cast<float>(value->array[1].NumberOr(field.y));
            }
        };
        read_vec2("windowPadding", theme.metrics.windowPadding);
        read_vec2("framePadding", theme.metrics.framePadding);
        read_vec2("itemSpacing", theme.metrics.itemSpacing);
        read_vec2("itemInnerSpacing", theme.metrics.itemInnerSpacing);
        read_vec2("cellPadding", theme.metrics.cellPadding);
        read_vec2("windowTitleAlign", theme.metrics.windowTitleAlign);
        read_vec2("buttonTextAlign", theme.metrics.buttonTextAlign);
        read_vec2("selectableTextAlign", theme.metrics.selectableTextAlign);
        read_float("windowRounding", theme.metrics.windowRounding);
        read_float("childRounding", theme.metrics.childRounding);
        read_float("frameRounding", theme.metrics.frameRounding);
        read_float("popupRounding", theme.metrics.popupRounding);
        read_float("scrollbarRounding", theme.metrics.scrollbarRounding);
        read_float("grabRounding", theme.metrics.grabRounding);
        read_float("tabRounding", theme.metrics.tabRounding);
        read_float("windowBorderSize", theme.metrics.windowBorderSize);
        read_float("childBorderSize", theme.metrics.childBorderSize);
        read_float("frameBorderSize", theme.metrics.frameBorderSize);
        read_float("popupBorderSize", theme.metrics.popupBorderSize);
        read_float("tabBorderSize", theme.metrics.tabBorderSize);
        read_float("scrollbarSize", theme.metrics.scrollbarSize);
        read_float("grabMinSize", theme.metrics.grabMinSize);
        read_float("indentSpacing", theme.metrics.indentSpacing);
        read_float("separatorTextBorder", theme.metrics.separatorTextBorder);
        read_float("windowMinWidth", theme.metrics.windowMinWidth);
        read_float("windowMinHeight", theme.metrics.windowMinHeight);
        read_float("fontSizeBase", theme.metrics.fontSizeBase);
        read_float("fontSizeSmall", theme.metrics.fontSizeSmall);
        read_float("fontSizeTitle", theme.metrics.fontSizeTitle);
        read_float("fontSizeMono", theme.metrics.fontSizeMono);
        read_float("rowHeight", theme.metrics.rowHeight);
        read_float("toolbarHeight", theme.metrics.toolbarHeight);
        read_float("statusBarHeight", theme.metrics.statusBarHeight);
        read_float("controlHeight", theme.metrics.controlHeight);
    }

    if (const Json::Value* fonts = root.Find("fonts"); fonts && fonts->IsObject())
    {
        auto read_string = [fonts](const char* key, std::string& field) {
            if (const Json::Value* value = fonts->Find(key); value && value->IsString())
                field = value->string;
        };
        read_string("regular", theme.fonts.regular);
        read_string("medium", theme.fonts.medium);
        read_string("bold", theme.fonts.bold);
        read_string("mono", theme.fonts.mono);
    }

    out_theme = theme;
    return true;
}

bool ThemeFromJson(const std::string& json, Theme& out_theme, std::string* error)
{
    return ThemeFromJson(json.data(), json.size(), out_theme, error);
}

bool SaveThemeToFile(const Theme& theme, const std::string& path, std::string* error)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        if (error) *error = "could not open file for writing: " + path;
        return false;
    }
    const std::string json = ThemeToJson(theme, true);
    file.write(json.data(), static_cast<std::streamsize>(json.size()));
    if (!file.good())
    {
        if (error) *error = "failed while writing: " + path;
        return false;
    }
    return true;
}

bool LoadThemeFromFile(const std::string& path, Theme& out_theme, std::string* error)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        if (error) *error = "could not open file: " + path;
        return false;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return ThemeFromJson(buffer.str(), out_theme, error);
}

std::string ThemeToCpp(const Theme& theme, const std::string& function_name)
{
    const std::string function = SanitizeIdentifier(function_name.empty() ? theme.name : function_name);
    std::string out;
    out += "// Generated by Amber UI. SPDX-License-Identifier: MIT\n";
    out += "#include <AmberUI/AmberUI.h>\n\n";
    out += "AmberUI::Theme " + function + "()\n{\n";
    out += "    AmberUI::Theme theme;\n";
    auto literal = [](const ImVec4& color) {
        char buffer[128];
        std::snprintf(buffer, sizeof(buffer), "ImVec4(%.4ff, %.4ff, %.4ff, %.4ff)", color.x, color.y, color.z, color.w);
        return std::string(buffer);
    };
    out += "    theme.name = \"" + theme.name + "\";\n";
    char buffer[128];
    std::snprintf(buffer, sizeof(buffer), "    theme.alpha = %.3ff;\n", theme.alpha);
    out += buffer;
    std::snprintf(buffer, sizeof(buffer), "    theme.dpiScale = %.3ff;\n", theme.dpiScale);
    out += buffer;
    for (int i = 0; i < kColorTokenCount; ++i)
    {
        out += "    theme.Color(AmberUI::ColorToken::";
        out += ColorTokenName(static_cast<ColorToken>(i));
        out += ") = " + literal(theme.colors[static_cast<size_t>(i)]) + ";\n";
    }

    auto emit_float = [&out](const char* field, float value) {
        char line[128];
        std::snprintf(line, sizeof(line), "    theme.metrics.%s = %.3ff;\n", field, value);
        out += line;
    };
    auto emit_vec2 = [&out](const char* field, const ImVec2& value) {
        char line[160];
        std::snprintf(line, sizeof(line), "    theme.metrics.%s = ImVec2(%.3ff, %.3ff);\n", field, value.x, value.y);
        out += line;
    };
    emit_vec2("windowPadding", theme.metrics.windowPadding);
    emit_vec2("framePadding", theme.metrics.framePadding);
    emit_vec2("itemSpacing", theme.metrics.itemSpacing);
    emit_vec2("itemInnerSpacing", theme.metrics.itemInnerSpacing);
    emit_vec2("cellPadding", theme.metrics.cellPadding);
    emit_float("windowRounding", theme.metrics.windowRounding);
    emit_float("childRounding", theme.metrics.childRounding);
    emit_float("frameRounding", theme.metrics.frameRounding);
    emit_float("popupRounding", theme.metrics.popupRounding);
    emit_float("scrollbarRounding", theme.metrics.scrollbarRounding);
    emit_float("grabRounding", theme.metrics.grabRounding);
    emit_float("tabRounding", theme.metrics.tabRounding);
    emit_float("windowBorderSize", theme.metrics.windowBorderSize);
    emit_float("childBorderSize", theme.metrics.childBorderSize);
    emit_float("frameBorderSize", theme.metrics.frameBorderSize);
    emit_float("popupBorderSize", theme.metrics.popupBorderSize);
    emit_float("tabBorderSize", theme.metrics.tabBorderSize);
    emit_float("scrollbarSize", theme.metrics.scrollbarSize);
    emit_float("grabMinSize", theme.metrics.grabMinSize);
    emit_float("indentSpacing", theme.metrics.indentSpacing);
    emit_float("separatorTextBorder", theme.metrics.separatorTextBorder);
    emit_float("windowMinWidth", theme.metrics.windowMinWidth);
    emit_float("windowMinHeight", theme.metrics.windowMinHeight);
    emit_float("fontSizeBase", theme.metrics.fontSizeBase);
    emit_float("fontSizeSmall", theme.metrics.fontSizeSmall);
    emit_float("fontSizeTitle", theme.metrics.fontSizeTitle);
    emit_float("fontSizeMono", theme.metrics.fontSizeMono);
    emit_float("rowHeight", theme.metrics.rowHeight);
    emit_float("toolbarHeight", theme.metrics.toolbarHeight);
    emit_float("statusBarHeight", theme.metrics.statusBarHeight);
    emit_float("controlHeight", theme.metrics.controlHeight);
    out += "    theme.fonts.regular = \"" + theme.fonts.regular + "\";\n";
    out += "    theme.fonts.medium = \"" + theme.fonts.medium + "\";\n";
    out += "    theme.fonts.bold = \"" + theme.fonts.bold + "\";\n";
    out += "    theme.fonts.mono = \"" + theme.fonts.mono + "\";\n";
    out += "    return theme;\n}\n";
    return out;
}

bool ImportThemeFromJson(const std::string& json, std::string* error)
{
    Theme theme;
    if (!ThemeFromJson(json, theme, error))
        return false;
    if (theme.name.empty())
    {
        if (error) *error = "theme JSON is missing a name";
        return false;
    }
    ThemeManager::Get().RegisterTheme(theme);
    return true;
}

} // namespace AmberUI
