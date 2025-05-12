#include "compiler.h"
#include "parser.h"
#include "scanner.h"
#include <iostream>

std::shared_ptr<Chunk> Compiler::compile(const std::string &source) {
  parser_ = std::make_unique<Parser>(source);
  parser_->advance();
  return compilingChunk_;
}

void Compiler::emitByte(OpCode op) {
  compilingChunk_->Write(to_underlying(op), parser_->previous().line);
}

void Compiler::emitBytes(OpCode op1, OpCode op2) {
  emitByte(op1);
  emitByte(op2);
}

void Compiler::emitReturn() { emitByte(OpCode::Return); }