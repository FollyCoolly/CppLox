#include "debug.h"
#include "object.h"
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
  std::cout << std::format("{:<16} {:>4} '{}' \n", name, constantIdx,
                           chunk.constants[constantIdx]);
  return offset + 2;
}

int byteInstruction(std::string_view name, const Chunk &chunk, int offset) {
  uint8_t slot = chunk.code[offset + 1];
  std::cout << std::format("{:<16} {:>4}\n", name, slot);
  return offset + 2;
}

int jumpInstruction(std::string_view name, const Chunk &chunk, int sign,
                    int offset) {
  uint16_t jump = static_cast<uint16_t>(chunk.code[offset + 1]) << 8 |
                  (chunk.code[offset + 2]);
  std::cout << std::format("{:<16} {:>4} -> {}\n", name, offset,
                           offset + 3 + sign * jump);
  return offset + 3;
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
  case OpCode::FALSE:
    return simpleInstruction("OP_FALSE", offset);
  case OpCode::TRUE:
    return simpleInstruction("OP_TRUE", offset);
  case OpCode::NIL:
    return simpleInstruction("OP_NIL", offset);
  case OpCode::PRINT:
    return simpleInstruction("OP_PRINT", offset);
  case OpCode::POP:
    return simpleInstruction("OP_POP", offset);
  case OpCode::DEFINE_GLOBAL:
    return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
  case OpCode::GET_GLOBAL:
    return constantInstruction("OP_GET_GLOBAL", chunk, offset);
  case OpCode::SET_GLOBAL:
    return constantInstruction("OP_SET_GLOBAL", chunk, offset);
  case OpCode::GET_LOCAL:
    return byteInstruction("OP_GET_LOCAL", chunk, offset);
  case OpCode::SET_LOCAL:
    return byteInstruction("OP_SET_LOCAL", chunk, offset);
  case OpCode::JUMP_IF_FALSE:
    return jumpInstruction("OP_JUMP_IF_FALSE", chunk, 1, offset);
  case OpCode::JUMP:
    return jumpInstruction("OP_JUMP", chunk, 1, offset);
  case OpCode::LOOP:
    return jumpInstruction("OP_LOOP", chunk, -1, offset);
  case OpCode::CALL:
    return byteInstruction("OP_CALL", chunk, offset);
  case OpCode::CLOSURE: {
    offset++;
    uint8_t constantIdx = chunk.code[offset++];
    std::cout << std::format("{:<16} {:>4}\n", "OP_CLOSURE", constantIdx);
    std::cout << chunk.constants[constantIdx] << std::endl;

    auto function = obj_helpers::AsFunction(chunk.constants[constantIdx]);
    for (int i = 0; i < function->upvalueCount; i++) {
      int isLocal = chunk.code[offset++];
      int index = chunk.code[offset++];
      std::cout << std::format("{:04d}      |                     {} {}\n",
                               offset, isLocal ? "local" : "upvalue", index);
    }

    return offset;
  }
  case OpCode::CLOSE_UPVALUE:
    return simpleInstruction("OP_CLOSE_UPVALUE", offset);
  case OpCode::GET_UPVALUE:
    return byteInstruction("OP_GET_UPVALUE", chunk, offset);
  case OpCode::SET_UPVALUE:
    return byteInstruction("OP_SET_UPVALUE", chunk, offset);
  default:
    std::cout << std::format("Unknown opcode {}\n", instruction);
    return offset + 1;
  }
}
