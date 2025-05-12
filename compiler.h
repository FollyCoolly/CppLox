#pragma once

#include "chunk.h"
#include "parser.h"
#include <memory>
#include <string>

class Compiler {
public:
  std::shared_ptr<Chunk> compile(const std::string &source);
  void endCompiler();

  void emitByte(OpCode op);
  void emitByte(uint8_t byte);
  void emitBytes(OpCode op, uint8_t byte);
  void emitReturn();
  void emitConstant(Value value);
  uint8_t makeConstant(Value value);

private:
  std::shared_ptr<Chunk> compilingChunk_;
  std::unique_ptr<Parser> parser_;
};