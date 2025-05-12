#pragma once

#include "chunk.h"
#include "parser.h"
#include <memory>
#include <string>

class Compiler {
public:
  std::shared_ptr<Chunk> compile(const std::string &source);
  void emitByte(OpCode op);
  void emitByte(uint8_t byte);
  void emitBytes(OpCode op, uint8_t byte);
  void emitReturn();
  void endCompiler();

private:
  std::shared_ptr<Chunk> compilingChunk_;
  std::unique_ptr<Parser> parser_;
};