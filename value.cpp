#include "value.h"
#include "object.h"
#include <format>
#include <iostream>

template <> struct std::formatter<Value> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const Value &value, FormatContext &ctx) const {
    switch (value.type) {
    case Value::Type::BOOL:
      return std::format_to(ctx.out(), "{}",
                            value.as.boolean ? "true" : "false");
    case Value::Type::NIL:
      return std::format_to(ctx.out(), "nil");
    case Value::Type::NUMBER:
      return std::format_to(ctx.out(), "{}", value.as.number);
    case Value::Type::OBJECT:
      return std::format_to(ctx.out(), "{}", *value.as.obj);
    }
    return ctx.out();
  }
};

std::ostream &operator<<(std::ostream &os, const Value &value) {
  switch (value.type) {
  case Value::Type::BOOL:
    os << (value.as.boolean ? "true" : "false");
    break;
  case Value::Type::NIL:
    os << "nil";
    break;
  case Value::Type::NUMBER:
    os << value.as.number;
    break;
  case Value::Type::OBJECT:
    os << *value.as.obj;
    break;
  }
  return os;
}

bool Value::operator==(const Value &other) const {
  if (type != other.type)
    return false;
  switch (type) {
  case Type::BOOL:
    return as.boolean == other.as.boolean;
  case Type::NIL:
    return true;
  case Type::NUMBER:
    return as.number == other.as.number;
  case Type::OBJECT: {
    // TODO: support other object types
    ObjString *obj1 = obj_helpers::AsString(*this);
    ObjString *obj2 = obj_helpers::AsString(other);
    return obj1->str == obj2->str;
  }
  default:
    return false; // unreachable
  }
}