#include "common.h"

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
