#include "compiler.h"
#include "parser.h"
#include "scanner.h"
#include <iostream>

std::shared_ptr<Chunk> Compiler::compile(const std::string &source) {
  parser_ = std::make_unique<Parser>(source);
  parser_->advance();
  return compilingChunk_;
}

void Compiler::endCompiler() { emitReturn(); }

void Compiler::emitByte(OpCode op) {
  compilingChunk_->Write(to_underlying(op), parser_->previous().line);
}

void Compiler::emitByte(uint8_t byte) {
  compilingChunk_->Write(byte, parser_->previous().line);
}

void Compiler::emitBytes(OpCode op, uint8_t byte) {
  emitByte(op);
  emitByte(byte);
}

void Compiler::emitReturn() { emitByte(OpCode::Return); }

void Compiler::emitConstant(Value value) {
  emitBytes(OpCode::Constant, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
  auto constantIndex = compilingChunk_->AddConstant(value);
  if (constantIndex >= UINT8_MAX) {
    parser_->error("Too many constants in one chunk.");
    return 0;
  }
  return static_cast<uint8_t>(constantIndex);
}
