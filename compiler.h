#pragma once

#include "chunk.h"
#include "parser.h"
#include <memory>
#include <string>

class Compiler {
public:
  std::shared_ptr<Chunk> compile(const std::string &source);
  void emitByte(OpCode op);
  void emitBytes(OpCode op1, OpCode op2);
  void emitReturn();

private:
  std::shared_ptr<Chunk> compilingChunk_;
  std::unique_ptr<Parser> parser_;
};