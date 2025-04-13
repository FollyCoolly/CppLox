#include "chunk.h"
#include "memory.h"

void Chunk::Write(OpCode op, int line) { Write(to_underlying(op), line); }

void Chunk::Write(uint8_t byte, int line) {
  if (capacity < count + 1) {
    int old_capacity = capacity;
    capacity = GROW_CAPACITY(old_capacity);
    code = GROW_ARRAY(uint8_t, code, old_capacity, capacity);
    lines = GROW_ARRAY(int, lines, old_capacity, capacity);
  }

  code[count] = byte;
  lines[count] = line;
  count++;
}

int Chunk::AddConstant(Value value) {
  constants.Write(value);
  return constants.count - 1;
}
