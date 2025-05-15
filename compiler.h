#pragma once

#include "chunk.h"
#include "parser.h"
#include <memory>
#include <string>
#include <sys/types.h>

enum class Precedence : uint8_t {
  NONE,
  ASSIGNMENT,
  OR,
  AND,
  EQUALITY,
  COMPARISON,
  TERM,
  FACTOR,
  UNARY,
  CALL,
  PRIMARY
};

class Compiler;

struct ParseRule {
  using ParseFn = void (*)(Compiler *);
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

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

  static void parsePrecedence(Compiler *compiler, Precedence precedence);
  static const ParseRule *getRule(TokenType type);

  static void expression(Compiler *compiler);
  static void grouping(Compiler *compiler);
  static void unary(Compiler *compiler);
  static void binary(Compiler *compiler);
  static void number(Compiler *compiler);

private:
  std::shared_ptr<Chunk> compilingChunk_;
  std::unique_ptr<Parser> parser_;
};
