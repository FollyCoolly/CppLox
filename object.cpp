#include "object.h"
#include "value.h"
#include <format>
#include <iostream>

template <> struct std::formatter<Obj> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const Obj &obj, FormatContext &ctx) const {
    switch (obj.type) {
    case Obj::Type::STRING:
      return std::format_to(ctx.out(), "{}",
                            static_cast<const ObjString &>(obj).str);
    }
    return ctx.out();
  }
};

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