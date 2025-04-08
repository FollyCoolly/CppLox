#pragma once

#include "memory.h"

using Value = double;

struct ValueArray {
  Value *values;
  int capacity;
  int count;

  ValueArray() : values(nullptr), capacity(0), count(0) {}
  ~ValueArray() { FREE_ARRAY(Value, values, capacity); }

  void Write(Value value);
};
