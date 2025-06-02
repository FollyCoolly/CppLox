#pragma once

#include "common.h"
#include "value.h"
#include <string>
#include <string_view>

struct Obj {
  enum class Type {
    STRING,
  };

  Type type;
};

struct ObjString : Obj {
  std::string str;

  template <typename T>
  ObjString(T &&str) : Obj{Type::STRING}, str(std::forward<T>(str)) {}

  ObjString(const char *chars, int length)
      : Obj{Type::STRING}, str(chars, length) {}
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
} // namespace obj_helpers