#pragma once

#include "common.h"
#include "memory.h"
enum class OpCode {
  Return,
};

struct Chunk {
  uint8_t *code_;
  int count_;
  int capacity_;

  Chunk() : code_(nullptr), count_(0), capacity_(0) {}
  ~Chunk() { FREE_ARRAY(uint8_t, code_, capacity_); }

  void Write(uint8_t byte);
};
