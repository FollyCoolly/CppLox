#pragma once

#include "chunk.h"
#include "object.h"
#include "parser.h"
#include "scanner.h"
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

enum class FunctionType {
  FUNCTION,
  SCRIPT,
};

class Compiler;

struct ParseRule {
  using ParseFn = void (*)(Compiler *, bool canAssign);
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

struct Local {
  Token name;
  int depth;
};

class Compiler {
public:
  std::shared_ptr<ObjFunction> compile(const std::string &source);
  std::shared_ptr<ObjFunction> endCompiler();

  Chunk *currentChunk() { return compilingFunction_->chunk.get(); }

  void emitByte(OpCode op);
  void emitByte(uint8_t byte);
  void emitBytes(OpCode op, uint8_t byte);
  void emitBytes(OpCode op1, OpCode op2);
  void emitReturn();
  void emitConstant(Value value);
  uint8_t makeConstant(Value value);

  int emitJump(OpCode op);
  void patchJump(int offset);
  void emitLoop(int loopStart);

  void addLocal(const Token &name);
  int resolveLocal(const Token &name);
  uint8_t identifierConstant(const Token &name);
  void markInitialized();

  static void parsePrecedence(Compiler *compiler, Precedence precedence);
  static const ParseRule *getRule(TokenType type);

  static void synchronize(Compiler *compiler);
  static uint8_t parseVariable(Compiler *compiler,
                               const std::string &errorMessage);
  static void defineVariable(Compiler *compiler, uint8_t global);
  static void declareVariable(Compiler *compiler);

  static void declaration(Compiler *compiler);
  static void varDeclaration(Compiler *compiler);
  static void statement(Compiler *compiler);
  static void printStatement(Compiler *compiler);
  static void ifStatement(Compiler *compiler);
  static void expressionStatement(Compiler *compiler);
  static void expression(Compiler *compiler);
  static void grouping(Compiler *compiler, bool canAssign);
  static void unary(Compiler *compiler, bool canAssign);
  static void binary(Compiler *compiler, bool canAssign);
  static void number(Compiler *compiler, bool canAssign);
  static void literal(Compiler *compiler, bool canAssign);
  static void string(Compiler *compiler, bool canAssign);
  static void variable(Compiler *compiler, bool canAssign);
  static void namedVariable(Compiler *compiler, const Token &name,
                            bool canAssign);
  static void block(Compiler *compiler);
  static void logicalAnd(Compiler *compiler, bool canAssign);
  static void logicalOr(Compiler *compiler, bool canAssign);
  static void whileStatement(Compiler *compiler);
  static void forStatement(Compiler *compiler);

  static void beginScope(Compiler *compiler);
  static void endScope(Compiler *compiler);

private:
  std::shared_ptr<ObjFunction> compilingFunction_;
  FunctionType currentFunctionType_ = FunctionType::SCRIPT;
  std::unique_ptr<Parser> parser_;
  std::vector<Local> locals_;
  int scopeDepth_ = 0;
};
