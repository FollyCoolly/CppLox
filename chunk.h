#pragma once

#include "common.h"
#include "memory.h"

enum class OpCode : uint8_t {
  Return = 0,
};

constexpr uint8_t to_underlying(OpCode op) { return static_cast<uint8_t>(op); }

constexpr OpCode from_uint8(uint8_t value) {
  return static_cast<OpCode>(value);
}

struct Chunk {
  uint8_t *code;
  int count;
  int capacity;

  Chunk() : code(nullptr), count(0), capacity(0) {}
  ~Chunk() { FREE_ARRAY(uint8_t, code, capacity); }

  void Write(uint8_t byte);
  void Write(OpCode op);
};
