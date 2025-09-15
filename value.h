#pragma once

#include "common.h"
#include <format>
#include <iostream>
#include <memory>
#include <variant>

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
  std::variant<bool, std::monostate, double, std::shared_ptr<Obj>> data;

  bool operator==(const Value &other) const;

  static Value Bool(bool value) {
    Value v;
    v.type = Type::BOOL;
    v.data = value;
    return v;
  }
  static Value Nil() {
    Value v;
    v.type = Type::NIL;
    v.data = std::monostate{};
    return v;
  }
  static Value Number(double value) {
    Value v;
    v.type = Type::NUMBER;
    v.data = value;
    return v;
  }
  static Value Object(std::shared_ptr<Obj> value) {
    Value v;
    v.type = Type::OBJECT;
    v.data = value;
    return v;
  }

  static bool AsBool(const Value &value) { return std::get<bool>(value.data); }
  static double AsNumber(const Value &value) {
    return std::get<double>(value.data);
  }
  static Obj *AsObject(const Value &value) {
    return std::get<std::shared_ptr<Obj>>(value.data).get();
  }

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
                            std::get<bool>(value.data) ? "true" : "false");
    case Value::Type::NIL:
      return std::format_to(ctx.out(), "nil");
    case Value::Type::NUMBER:
      return std::format_to(ctx.out(), "{}", std::get<double>(value.data));
    case Value::Type::OBJECT:
      return std::format_to(ctx.out(), "<object>");
    }
    return ctx.out();
  }
};

std::ostream &operator<<(std::ostream &os, const Value &value);
