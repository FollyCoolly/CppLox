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

using NativeFunction = Value (*)(int argCount, Value *args);

struct Obj {
  enum class Type {
    STRING,
    FUNCTION,
    NATIVE,
    CLOSURE,
    UPVALUE,
    CLASS,
    INSTANCE,
  };

  Type type;
};

struct ObjString : Obj {
  std::string str;

  static std::shared_ptr<ObjString> getObject(const char *chars, int length) {
    static std::unordered_map<std::string_view, std::shared_ptr<ObjString>>
        interned_strings;
    static std::vector<std::string> string_storage;

    std::string_view key(chars, length);
    auto it = interned_strings.find(key);
    if (it != interned_strings.end()) {
      return it->second;
    }

    string_storage.emplace_back(chars, length);
    key = string_storage.back();

    auto obj = std::make_shared<ObjString>(key);
    interned_strings[key] = obj;
    return obj;
  }

  static std::shared_ptr<ObjString> getObject(const std::string &str) {
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
  int upvalueCount;
  std::shared_ptr<Chunk> chunk;
  ObjString *name;

  ObjFunction(int arity, ObjString *name)
      : Obj{Type::FUNCTION}, arity(arity), upvalueCount(0),
        chunk(std::make_unique<Chunk>()), name(name) {}
};

struct ObjNative : Obj {
  NativeFunction function;

  ObjNative(NativeFunction function) : Obj{Type::NATIVE}, function(function) {}
};

struct ObjUpvalue : Obj {
  Value closed;
  int stackIdx;

  ObjUpvalue(int stackIdx) : Obj{Type::UPVALUE}, stackIdx(stackIdx) {}
};

struct ObjClosure : Obj {
  int upvalueCount;
  std::vector<std::shared_ptr<ObjUpvalue>> upvalues;
  ObjFunction *function;

  ObjClosure(ObjFunction *function)
      : Obj{Type::CLOSURE}, function(function),
        upvalueCount(function->upvalueCount), upvalues(upvalueCount) {}
};

struct ObjClass : Obj {
  ObjString *name;

  ObjClass(ObjString *name) : Obj{Type::CLASS}, name(name) {}
};

struct ObjInstance : Obj {
  ObjClass *klass;
  std::unordered_map<std::string, Value> fields;

  ObjInstance(ObjClass *klass) : Obj{Type::INSTANCE}, klass(klass) {}
};

namespace obj_helpers {
inline bool IsObjType(const Value &value, Obj::Type type) {
  return value.type == Value::Type::OBJECT &&
         std::get<std::shared_ptr<Obj>>(value.data)->type == type;
}

inline bool IsString(const Value &value) {
  return IsObjType(value, Obj::Type::STRING);
}

inline bool IsNative(const Value &value) {
  return IsObjType(value, Obj::Type::NATIVE);
}

inline bool IsClosure(const Value &value) {
  return IsObjType(value, Obj::Type::CLOSURE);
}

inline bool IsFunction(const Value &value) {
  return IsObjType(value, Obj::Type::FUNCTION);
}

inline bool IsUpvalue(const Value &value) {
  return IsObjType(value, Obj::Type::UPVALUE);
}

inline bool IsClass(const Value &value) {
  return IsObjType(value, Obj::Type::CLASS);
}

inline bool IsInstance(const Value &value) {
  return IsObjType(value, Obj::Type::INSTANCE);
}

inline ObjString *AsString(const Value &value) {
  return static_cast<ObjString *>(Value::AsObject(value));
}

inline ObjFunction *AsFunction(const Value &value) {
  return static_cast<ObjFunction *>(Value::AsObject(value));
}

inline NativeFunction AsNative(const Value &value) {
  return static_cast<ObjNative *>(Value::AsObject(value))->function;
}

inline ObjClosure *AsClosure(const Value &value) {
  return static_cast<ObjClosure *>(Value::AsObject(value));
}

inline ObjUpvalue *AsUpvalue(const Value &value) {
  return static_cast<ObjUpvalue *>(Value::AsObject(value));
}

inline ObjClass *AsClass(const Value &value) {
  return static_cast<ObjClass *>(Value::AsObject(value));
}

inline ObjInstance *AsInstance(const Value &value) {
  return static_cast<ObjInstance *>(Value::AsObject(value));
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
    case Obj::Type::NATIVE:
      return std::format_to(ctx.out(), "<native fn>");
    case Obj::Type::CLOSURE:
      return std::format_to(
          ctx.out(), "<closure {}>",
          static_cast<const ObjClosure &>(obj).function->name->str);
    case Obj::Type::UPVALUE:
      return std::format_to(ctx.out(), "<upvalue>");
    case Obj::Type::CLASS:
      return std::format_to(ctx.out(), "<class {}>",
                            static_cast<const ObjClass &>(obj).name->str);
    case Obj::Type::INSTANCE:
      return std::format_to(
          ctx.out(), "<{} instance>",
          static_cast<const ObjInstance &>(obj).klass->name->str);
    }
    return ctx.out();
  }
};

std::ostream &operator<<(std::ostream &os, const Obj &obj);
