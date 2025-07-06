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
    os << "<fn " << static_cast<const ObjFunction &>(obj).name->str << ">";
    break;
  }
  return os;
}
