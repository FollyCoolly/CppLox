#include "object.h"
#include "value.h"
#include <format>
#include <iostream>

std::ostream &operator<<(std::ostream &os, const Obj &obj) {
  switch (obj.type) {
  case Obj::Type::STRING:
    os << static_cast<const ObjString &>(obj).str;
    break;
  case Obj::Type::FUNCTION:
    if (static_cast<const ObjFunction &>(obj).name != nullptr) {
      os << "<fn " << static_cast<const ObjFunction &>(obj).name->str << ">";
    } else {
      os << "<script>";
    }
    break;
  }
  return os;
}
