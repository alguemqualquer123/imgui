// Amber UI - Minimal internal JSON reader/writer
// SPDX-License-Identifier: MIT
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace AmberUI
{
namespace Json
{
struct Value
{
    enum class Type
    {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    Type type = Type::Null;
    bool boolean = false;
    double number = 0.0;
    std::string string;
    std::vector<Value> array;
    std::vector<std::pair<std::string, Value>> object;

    bool IsNull() const { return type == Type::Null; }
    bool IsObject() const { return type == Type::Object; }
    bool IsArray() const { return type == Type::Array; }
    bool IsNumber() const { return type == Type::Number; }
    bool IsString() const { return type == Type::String; }

    const Value* Find(const char* key) const;
    double NumberOr(double fallback) const;
    const std::string& StringOr(const std::string& fallback) const;
};

bool Parse(const char* text, size_t length, Value& out, std::string& error);
std::string Serialize(const Value& value, bool pretty);

// Convenience builders.
Value MakeObject();
Value MakeString(const std::string& value);
Value MakeNumber(double value);
Value MakeBool(bool value);
Value MakeArray();
void Set(Value& object, const char* key, Value value);
} // namespace Json
} // namespace AmberUI
