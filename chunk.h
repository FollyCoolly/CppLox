#pragma once

#include "common.h"
#include "value.h"
#include <vector>

enum class OpCode : uint8_t {
  CONSTANT,
  RETURN,
  NEGATE,
  ADD,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  FALSE,
  TRUE,
  NIL,
  NOT,
  EQUAL,
  GREATER,
  LESS,
  PRINT,
  POP,
};

constexpr uint8_t to_underlying(OpCode op) { return static_cast<uint8_t>(op); }

constexpr OpCode from_uint8(uint8_t value) {
  return static_cast<OpCode>(value);
}

struct Chunk {
  std::vector<uint8_t> code;
  std::vector<Value> constants;
  std::vector<int> lines;

  Chunk() = default;
  ~Chunk() = default;

  void Write(uint8_t byte, int line);
  void Write(OpCode op, int line);

  int AddConstant(Value value);
};