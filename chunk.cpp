#include "chunk.h"

void Chunk::Write(OpCode op, int line) { Write(to_underlying(op), line); }

void Chunk::Write(uint8_t byte, int line) {
  code.push_back(byte);
  lines.push_back(line);
}

int Chunk::AddConstant(Value value) {
  constants.push_back(value);
  return constants.size() - 1;
}
