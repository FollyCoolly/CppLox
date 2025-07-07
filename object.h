#pragma once

#include "chunk.h"
#include "common.h"
#include "value.h"
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

struct Obj {
  enum class Type {
    STRING,
    FUNCTION,
  };

  Type type;
};

struct ObjString : Obj {
  std::string str;

  static ObjString *getObject(const char *chars, int length) {
    static std::unordered_map<std::string_view, std::unique_ptr<ObjString>>
        interned_strings;
    static std::vector<std::string> string_storage;

    std::string_view key(chars, length);
    auto it = interned_strings.find(key);
    if (it != interned_strings.end()) {
      return it->second.get();
    }

    string_storage.emplace_back(chars, length);
    key = string_storage.back();

    auto obj = std::make_unique<ObjString>(key);
    ObjString *result = obj.get();
    interned_strings[key] = std::move(obj);
    return result;
  }

  static ObjString *getObject(const std::string &str) {
    return getObject(str.c_str(), str.length());
  }

  ObjString(std::string_view str) : Obj{Type::STRING}, str(str) {}

private:
  ObjString() = delete;
  ObjString(const ObjString &) = delete;
  ObjString &operator=(const ObjString &) = delete;
};

struct ObjFunction : Obj {
  int arity;
  std::shared_ptr<Chunk> chunk;
  ObjString *name;

  ObjFunction(int arity, ObjString *name)
      : Obj{Type::FUNCTION}, arity(arity), chunk(std::make_unique<Chunk>()),
        name(name) {}
};

namespace obj_helpers {
inline bool IsObjType(const Value &value, Obj::Type type) {
  return value.type == Value::Type::OBJECT && value.as.obj->type == type;
}

inline bool IsString(const Value &value) {
  return IsObjType(value, Obj::Type::STRING);
}

inline ObjString *AsString(const Value &value) {
  return static_cast<ObjString *>(Value::AsObject(value));
}

inline ObjFunction *AsFunction(const Value &value) {
  return static_cast<ObjFunction *>(Value::AsObject(value));
}

inline ObjString *AsObjString(const Value &value) {
  return static_cast<ObjString *>(Value::AsObject(value));
}
} // namespace obj_helpers

template <> struct std::formatter<Obj> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const Obj &obj, FormatContext &ctx) const {
    switch (obj.type) {
    case Obj::Type::STRING:
      return std::format_to(ctx.out(), "{}",
                            static_cast<const ObjString &>(obj).str);
    case Obj::Type::FUNCTION:
      if (static_cast<const ObjFunction &>(obj).name != nullptr) {
        return std::format_to(ctx.out(), "<fn {}>",
                              static_cast<const ObjFunction &>(obj).name->str);
      }
      return std::format_to(ctx.out(), "<script>");
    }
    return ctx.out();
  }
};

std::ostream &operator<<(std::ostream &os, const Obj &obj);
