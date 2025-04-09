#pragma once

#include "common.h"
#include "memory.h"
#include "value.h"

enum class OpCode : uint8_t { Constant, Return };

constexpr uint8_t to_underlying(OpCode op) { return static_cast<uint8_t>(op); }

constexpr OpCode from_uint8(uint8_t value) {
  return static_cast<OpCode>(value);
}

struct Chunk {
  uint8_t *code;
  ValueArray constants;
  int count;
  int capacity;

  Chunk() : code(nullptr), count(0), capacity(0) {}
  ~Chunk() { FREE_ARRAY(uint8_t, code, capacity); }

  void Write(uint8_t byte);
  void Write(OpCode op);

  int AddConstant(Value value);
};
