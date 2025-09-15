#include "object.h"
#include "value.h"
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
  case Obj::Type::CLOSURE:
    os << static_cast<const ObjClosure &>(obj);
    break;
  case Obj::Type::NATIVE:
    os << "<native fn>";
    break;
  case Obj::Type::UPVALUE:
    os << "<upvalue>";
    break;
  case Obj::Type::CLASS:
    os << "<class>";
    break;
  }
  return os;
}
