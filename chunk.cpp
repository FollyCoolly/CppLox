#include "chunk.h"

void Chunk::Write(OpCode op) { Write(to_underlying(op)); }

void Chunk::Write(uint8_t byte) {
  if (capacity < count + 1) {
    int old_capacity = capacity;
    capacity = GROW_CAPACITY(old_capacity);
    code = GROW_ARRAY(uint8_t, code, old_capacity, capacity);
  }

  code[count] = byte;
  count++;
}

int Chunk::AddConstant(Value value) {
  constants.Write(value);
  return constants.count - 1;
}
