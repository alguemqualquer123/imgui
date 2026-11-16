// Amber UI - Minimal internal JSON reader/writer
// SPDX-License-Identifier: MIT
#include "Serialization/Json.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace AmberUI
{
namespace Json
{
namespace
{
struct Parser
{
    const char* cursor = nullptr;
    const char* end = nullptr;
    std::string error;

    void SkipWhitespace()
    {
        while (cursor < end && (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r'))
            ++cursor;
    }

    bool Fail(const char* message)
    {
        if (error.empty())
            error = message;
        return false;
    }

    bool ParseValue(Value& out)
    {
        SkipWhitespace();
        if (cursor >= end)
            return Fail("unexpected end of input");
        switch (*cursor)
        {
        case '{': return ParseObject(out);
        case '[': return ParseArray(out);
        case '"': out.type = Value::Type::String; return ParseString(out.string);
        case 't':
            if (end - cursor >= 4 && std::strncmp(cursor, "true", 4) == 0)
            {
                cursor += 4;
                out.type = Value::Type::Bool;
                out.boolean = true;
                return true;
            }
            return Fail("invalid literal");
        case 'f':
            if (end - cursor >= 5 && std::strncmp(cursor, "false", 5) == 0)
            {
                cursor += 5;
                out.type = Value::Type::Bool;
                out.boolean = false;
                return true;
            }
            return Fail("invalid literal");
        case 'n':
            if (end - cursor >= 4 && std::strncmp(cursor, "null", 4) == 0)
            {
                cursor += 4;
                out.type = Value::Type::Null;
                return true;
            }
            return Fail("invalid literal");
        default: return ParseNumber(out);
        }
    }

    bool ParseNumber(Value& out)
    {
        const char* start = cursor;
        if (cursor < end && (*cursor == '-' || *cursor == '+'))
            ++cursor;
        while (cursor < end && ((*cursor >= '0' && *cursor <= '9') || *cursor == '.' || *cursor == 'e' ||
                                *cursor == 'E' || *cursor == '-' || *cursor == '+'))
            ++cursor;
        if (start == cursor)
            return Fail("invalid number");
        char buffer[64];
        const size_t length = static_cast<size_t>(cursor - start);
        if (length >= sizeof(buffer))
            return Fail("number too long");
        std::memcpy(buffer, start, length);
        buffer[length] = '\0';
        char* parse_end = nullptr;
        const double parsed = std::strtod(buffer, &parse_end);
        if (parse_end == buffer || !std::isfinite(parsed))
            return Fail("invalid number");
        out.type = Value::Type::Number;
        out.number = parsed;
        return true;
    }

    bool ParseString(std::string& out)
    {
        if (cursor >= end || *cursor != '"')
            return Fail("expected string");
        ++cursor;
        out.clear();
        while (cursor < end)
        {
            const char c = *cursor++;
            if (c == '"')
                return true;
            if (c == '\\')
            {
                if (cursor >= end)
                    return Fail("bad escape");
                const char esc = *cursor++;
                switch (esc)
                {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;
                case 't': out += '\t'; break;
                case 'u':
                {
                    if (end - cursor < 4)
                        return Fail("bad unicode escape");
                    auto hex = [](char ch) -> int {
                        if (ch >= '0' && ch <= '9') return ch - '0';
                        if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
                        if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
                        return -1;
                    };
                    int code = 0;
                    for (int i = 0; i < 4; ++i)
                    {
                        const int value = hex(cursor[i]);
                        if (value < 0)
                            return Fail("bad unicode escape");
                        code = code * 16 + value;
                    }
                    cursor += 4;
                    // Crockford-style UTF-8 encoding of the BMP code point.
                    if (code < 0x80)
                        out += static_cast<char>(code);
                    else if (code < 0x800)
                    {
                        out += static_cast<char>(0xC0 | (code >> 6));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    }
                    else
                    {
                        out += static_cast<char>(0xE0 | (code >> 12));
                        out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    }
                    break;
                }
                default: return Fail("unknown escape");
                }
            }
            else
            {
                out += c;
            }
        }
        return Fail("unterminated string");
    }

    bool ParseArray(Value& out)
    {
        ++cursor; // '['
        out.type = Value::Type::Array;
        SkipWhitespace();
        if (cursor < end && *cursor == ']')
        {
            ++cursor;
            return true;
        }
        while (cursor < end)
        {
            Value element;
            if (!ParseValue(element))
                return false;
            out.array.push_back(std::move(element));
            SkipWhitespace();
            if (cursor < end && *cursor == ',')
            {
                ++cursor;
                continue;
            }
            if (cursor < end && *cursor == ']')
            {
                ++cursor;
                return true;
            }
            return Fail("expected ',' or ']'");
        }
        return Fail("unterminated array");
    }

    bool ParseObject(Value& out)
    {
        ++cursor; // '{'
        out.type = Value::Type::Object;
        SkipWhitespace();
        if (cursor < end && *cursor == '}')
        {
            ++cursor;
            return true;
        }
        while (cursor < end)
        {
            SkipWhitespace();
            std::string key;
            if (!ParseString(key))
                return false;
            SkipWhitespace();
            if (cursor >= end || *cursor != ':')
                return Fail("expected ':'");
            ++cursor;
            Value value;
            if (!ParseValue(value))
                return false;
            out.object.emplace_back(std::move(key), std::move(value));
            SkipWhitespace();
            if (cursor < end && *cursor == ',')
            {
                ++cursor;
                continue;
            }
            if (cursor < end && *cursor == '}')
            {
                ++cursor;
                return true;
            }
            return Fail("expected ',' or '}'");
        }
        return Fail("unterminated object");
    }
};

void EscapeString(const std::string& input, std::string& out)
{
    out += '"';
    for (const char c : input)
    {
        switch (c)
        {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20)
            {
                char buffer[8];
                std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned char>(c));
                out += buffer;
            }
            else
            {
                out += c;
            }
            break;
        }
    }
    out += '"';
}

void WriteNumber(double value, std::string& out)
{
    if (value == static_cast<double>(static_cast<long long>(value)))
        out += std::to_string(static_cast<long long>(value));
    else
    {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%.6g", value);
        out += buffer;
    }
}

void SerializeValue(const Value& value, std::string& out, bool pretty, int depth)
{
    const std::string indent(pretty ? static_cast<size_t>(depth + 1) * 2 : 0, ' ');
    const std::string close_indent(pretty ? static_cast<size_t>(depth) * 2 : 0, ' ');
    switch (value.type)
    {
    case Value::Type::Null: out += "null"; break;
    case Value::Type::Bool: out += value.boolean ? "true" : "false"; break;
    case Value::Type::Number: WriteNumber(value.number, out); break;
    case Value::Type::String: EscapeString(value.string, out); break;
    case Value::Type::Array:
        if (value.array.empty())
        {
            out += "[]";
            break;
        }
        out += '[';
        if (pretty) out += '\n';
        for (size_t i = 0; i < value.array.size(); ++i)
        {
            if (pretty) out += indent;
            SerializeValue(value.array[i], out, pretty, depth + 1);
            if (i + 1 < value.array.size()) out += ',';
            if (pretty) out += '\n';
        }
        if (pretty) out += close_indent;
        out += ']';
        break;
    case Value::Type::Object:
        if (value.object.empty())
        {
            out += "{}";
            break;
        }
        out += '{';
        if (pretty) out += '\n';
        for (size_t i = 0; i < value.object.size(); ++i)
        {
            if (pretty) out += indent;
            EscapeString(value.object[i].first, out);
            out += pretty ? ": " : ":";
            SerializeValue(value.object[i].second, out, pretty, depth + 1);
            if (i + 1 < value.object.size()) out += ',';
            if (pretty) out += '\n';
        }
        if (pretty) out += close_indent;
        out += '}';
        break;
    }
}
} // namespace

const Value* Value::Find(const char* key) const
{
    if (!key || type != Type::Object)
        return nullptr;
    for (const auto& pair : object)
    {
        if (pair.first == key)
            return &pair.second;
    }
    return nullptr;
}

double Value::NumberOr(double fallback) const
{
    return type == Type::Number ? number : fallback;
}

const std::string& Value::StringOr(const std::string& fallback) const
{
    return type == Type::String ? string : fallback;
}

bool Parse(const char* text, size_t length, Value& out, std::string& error)
{
    Parser parser;
    parser.cursor = text;
    parser.end = text + length;
    out = Value();
    if (!parser.ParseValue(out))
    {
        error = parser.error.empty() ? "invalid JSON" : parser.error;
        return false;
    }
    parser.SkipWhitespace();
    if (parser.cursor != parser.end)
    {
        error = "trailing characters after JSON value";
        return false;
    }
    return true;
}

std::string Serialize(const Value& value, bool pretty)
{
    std::string out;
    SerializeValue(value, out, pretty, 0);
    return out;
}

Value MakeObject() { Value v; v.type = Value::Type::Object; return v; }
Value MakeString(const std::string& value) { Value v; v.type = Value::Type::String; v.string = value; return v; }
Value MakeNumber(double value) { Value v; v.type = Value::Type::Number; v.number = value; return v; }
Value MakeBool(bool value) { Value v; v.type = Value::Type::Bool; v.boolean = value; return v; }
Value MakeArray() { Value v; v.type = Value::Type::Array; return v; }

void Set(Value& object, const char* key, Value value)
{
    for (auto& pair : object.object)
    {
        if (pair.first == key)
        {
            pair.second = std::move(value);
            return;
        }
    }
    object.object.emplace_back(key, std::move(value));
}

} // namespace Json
} // namespace AmberUI
