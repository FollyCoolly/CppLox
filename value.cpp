#include "value.h"
#include "object.h"
#include <format>
#include <iostream>

std::ostream &operator<<(std::ostream &os, const Value &value) {
  switch (value.type) {
  case Value::Type::BOOL:
    os << (std::get<bool>(value.data) ? "true" : "false");
    break;
  case Value::Type::NIL:
    os << "nil";
    break;
  case Value::Type::NUMBER:
    os << std::get<double>(value.data);
    break;
  case Value::Type::OBJECT:
    if (obj_helpers::IsString(value)) {
      os << obj_helpers::AsString(value)->str;
    } else {
      os << "<object>";
    }
    break;
  }
  return os;
}

bool Value::operator==(const Value &other) const {
  if (type != other.type)
    return false;
  switch (type) {
  case Type::BOOL:
    return std::get<bool>(data) == std::get<bool>(other.data);
  case Type::NIL:
    return true;
  case Type::NUMBER:
    return std::get<double>(data) == std::get<double>(other.data);
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