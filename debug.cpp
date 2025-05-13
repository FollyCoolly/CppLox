#include "debug.h"
#include <format>
#include <iostream>
#include <string_view>

namespace {
int simpleInstruction(std::string_view name, int offset) {
  std::cout << std::format("{}\n", name);
  return offset + 1;
}

int constantInstruction(std::string_view name, const Chunk &chunk, int offset) {
  uint8_t constantIdx = chunk.code[offset + 1];
  std::cout << std::format("{:<16} {:>4} '{:g}' \n", name, constantIdx,
                           chunk.constants[constantIdx]);
  return offset + 2;
}
} // namespace

void disassembleChunk(const Chunk &chunk, std::string_view name) {
  std::cout << std::format("== {} ==\n", name);

  for (int offset = 0; offset < chunk.code.size();) {
    offset = disassembleInstruction(chunk, offset);
  }
}

int disassembleInstruction(const Chunk &chunk, int offset) {
  std::cout << std::format("{:04} ", offset);
  if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1]) {
    std::cout << "   | ";
  } else {
    std::cout << std::format("{:4} ", chunk.lines[offset]);
  }

  uint8_t instruction = chunk.code[offset];
  switch (from_uint8(instruction)) {
  case OpCode::CONSTANT:
    return constantInstruction("OP_CONSTANT", chunk, offset);
  case OpCode::RETURN:
    return simpleInstruction("OP_RETURN", offset);
  case OpCode::NEGATE:
    return simpleInstruction("OP_NEGATE", offset);
  default:
    std::cout << std::format("Unknown opcode {}\n", instruction);
    return offset + 1;
  }
}
