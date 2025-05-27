#include "value.h"

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
  default:
    return false; // unreachable
  }
}