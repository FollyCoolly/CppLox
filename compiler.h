#pragma once

#include "chunk.h"
#include "object.h"
#include "parser.h"
#include "scanner.h"
#include <cstdint>
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
  METHOD,
  INITIALIZER,
};

class Compiler;

struct ParseRule {
  using ParseFn = void (*)(Compiler *, bool can_assign);
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

struct Local {
  Token name;
  int depth;
  bool is_captured;
};

struct Upvalue {
  uint8_t index;
  bool is_local;
};

struct CompileContext {
  int scope_depth;
  std::shared_ptr<ObjFunction> function;
  FunctionType function_type;
  std::vector<Local> locals;
  std::vector<Upvalue> upvalues;
};

struct ClassContext {
  ClassContext *enclosing;
  bool has_superclass = false;
};

class Compiler {
public:
  std::shared_ptr<ObjFunction> compile(const std::string &source);
  std::shared_ptr<ObjFunction> endCompiler();

  Chunk *currentChunk() { return contexts_.back().function->chunk.get(); }

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
  uint8_t identifierConstant(const Token &name);
  void markInitialized();

  int resolveUpvalue(int contextIdx, const Token &name);
  int resolveLocal(CompileContext &context, const Token &name);
  int addUpvalue(CompileContext &context, uint8_t index, bool is_local);

  static void parsePrecedence(Compiler *compiler, Precedence precedence);
  static const ParseRule *getRule(TokenType type);

  static void synchronize(Compiler *compiler);
  static uint8_t parseVariable(Compiler *compiler,
                               const std::string &errorMessage);
  static void defineVariable(Compiler *compiler, uint8_t global);
  static void declareVariable(Compiler *compiler);

  static void function(Compiler *compiler, FunctionType type);
  static void call(Compiler *compiler, bool can_assign);
  static uint8_t argumentList(Compiler *compiler);

  static void declaration(Compiler *compiler);
  static void varDeclaration(Compiler *compiler);
  static void functionDeclaration(Compiler *compiler);
  static void classDeclaration(Compiler *compiler);

  static void method(Compiler *compiler);
  static void statement(Compiler *compiler);
  static void printStatement(Compiler *compiler);
  static void ifStatement(Compiler *compiler);
  static void returnStatement(Compiler *compiler);
  static void expressionStatement(Compiler *compiler);
  static void expression(Compiler *compiler);
  static void dot(Compiler *compiler, bool can_assign);
  static void grouping(Compiler *compiler, bool can_assign);
  static void unary(Compiler *compiler, bool can_assign);
  static void binary(Compiler *compiler, bool can_assign);
  static void number(Compiler *compiler, bool can_assign);
  static void literal(Compiler *compiler, bool can_assign);
  static void string(Compiler *compiler, bool can_assign);
  static void variable(Compiler *compiler, bool can_assign);
  static void namedVariable(Compiler *compiler, const Token &name,
                            bool can_assign);
  static void handleThis(Compiler *compiler, bool can_assign);
  static void handleSuper(Compiler *compiler, bool can_assign);
  static void block(Compiler *compiler);
  static void logicalAnd(Compiler *compiler, bool can_assign);
  static void logicalOr(Compiler *compiler, bool can_assign);
  static void whileStatement(Compiler *compiler);
  static void forStatement(Compiler *compiler);

  static void beginScope(Compiler *compiler);
  static void endScope(Compiler *compiler);

private:
  std::vector<CompileContext> contexts_;
  std::unique_ptr<Parser> parser_;
  ClassContext *current_class_;
};
