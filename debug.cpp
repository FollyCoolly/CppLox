#include "debug.h"
#include <iomanip>
#include <iostream>
#include <string_view>

namespace {
int simpleInstruction(std::string_view name, int offset) {
  std::cout << name << "\n";
  return offset + 1;
}
} // namespace

void disassembleChunk(const Chunk &chunk, std::string_view name) {
  std::cout << "== " << name << " ==\n";

  for (int offset = 0; offset < chunk.count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

int disassembleInstruction(const Chunk &chunk, int offset) {
  std::cout << std::setw(4) << std::setfill('0') << offset << " "
            << std::setw(0) << std::setfill(' ');

  uint8_t instruction = chunk.code[offset];
  switch (from_uint8(instruction)) {
  case OpCode::Return:
    return simpleInstruction("OP_RETURN", offset);
  default:
    std::cout << "Unknown opcode " << instruction << "\n";
    return offset + 1;
  }
}
