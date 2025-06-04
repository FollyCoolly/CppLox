#pragma once

#include "common.h"
#include <format>
#include <iostream>

struct Obj;
struct ObjString;

struct Value {
  enum class Type {
    BOOL,
    NIL,
    NUMBER,
    OBJECT,
  };

  Type type;
  union {
    bool boolean;
    double number;
    Obj *obj;
  } as;

  bool operator==(const Value &other) const;

  static Value Bool(bool value) {
    return Value(Type::BOOL, {.boolean = value});
  }
  static Value Nil() { return Value(Type::NIL, {.number = 0}); }
  static Value Number(double value) {
    return Value(Type::NUMBER, {.number = value});
  }
  static Value Object(Obj *value) {
    return Value(Type::OBJECT, {.obj = value});
  }

  static bool AsBool(const Value &value) { return value.as.boolean; }
  static double AsNumber(const Value &value) { return value.as.number; }
  static Obj *AsObject(const Value &value) { return value.as.obj; }

  static bool IsBool(const Value &value) { return value.type == Type::BOOL; }
  static bool IsNil(const Value &value) { return value.type == Type::NIL; }
  static bool IsNumber(const Value &value) {
    return value.type == Type::NUMBER;
  }
  static bool IsObject(const Value &value) {
    return value.type == Type::OBJECT;
  }
};

// Forward declarations
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
      return std::format_to(ctx.out(), "<object>");
    }
    return ctx.out();
  }
};

std::ostream &operator<<(std::ostream &os, const Value &value);
