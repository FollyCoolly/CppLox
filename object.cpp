#include "object.h"
#include "value.h"
#include <format>
#include <iostream>

std::ostream &operator<<(std::ostream &os, const Obj &obj) {
  switch (obj.type) {
  case Obj::Type::STRING:
    os << static_cast<const ObjString &>(obj).str;
    break;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const ObjString &obj) {
  os << obj.str;
  return os;
}