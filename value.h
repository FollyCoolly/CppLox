#include "common.h"
#include <format>
#include <iostream>

struct Value {
  enum class Type {
    BOOL,
    NIL,
    NUMBER,
  };

  Type type;
  union {
    bool boolean;
    double number;
  } as;

  bool operator==(const Value &other) const;

  static Value Bool(bool value) {
    return Value(Type::BOOL, {.boolean = value});
  }
  static Value Nil() { return Value(Type::NIL, {.number = 0}); }
  static Value Number(double value) {
    return Value(Type::NUMBER, {.number = value});
  }

  static bool AsBool(const Value &value) { return value.as.boolean; }
  static double AsNumber(const Value &value) { return value.as.number; }

  static bool IsBool(const Value &value) { return value.type == Type::BOOL; }
  static bool IsNil(const Value &value) { return value.type == Type::NIL; }
  static bool IsNumber(const Value &value) {
    return value.type == Type::NUMBER;
  }
};

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
    }
    return ctx.out();
  }
};

inline std::ostream &operator<<(std::ostream &os, const Value &value) {
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
  }
  return os;
}
