#include "compiler.h"

#include <cstdint>
#include <unordered_map>

#include "chunk.h"
#include "object.h"
#include "parser.h"
#include "scanner.h"
#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

namespace {
bool identifiersEqual(const Token &a, const Token &b) {
  return a.length == b.length && std::memcmp(a.start, b.start, a.length) == 0;
}
} // namespace

std::shared_ptr<ObjFunction> Compiler::compile(const std::string &source) {
  parser_ = std::make_unique<Parser>(source);
  compilingFunction_ = std::make_shared<ObjFunction>(0, nullptr);
  parser_->advance();
  while (!parser_->match(TokenType::END_OF_FILE)) {
    declaration(this);
  }
  auto function = endCompiler();
  if (parser_->hadError()) {
    return nullptr;
  }
  return function;
}

std::shared_ptr<ObjFunction> Compiler::endCompiler() {
  emitReturn();
#ifdef DEBUG_PRINT_CODE
  disassembleChunk(*currentChunk(), compilingFunction_->name != nullptr
                                        ? compilingFunction_->name->str
                                        : "<script>");
#endif
  return compilingFunction_;
}

void Compiler::emitByte(OpCode op) {
  currentChunk()->Write(to_underlying(op), parser_->previous().line);
}

void Compiler::emitByte(uint8_t byte) {
  currentChunk()->Write(byte, parser_->previous().line);
}

void Compiler::emitBytes(OpCode op, uint8_t byte) {
  emitByte(op);
  emitByte(byte);
}

void Compiler::emitBytes(OpCode op1, OpCode op2) {
  emitByte(op1);
  emitByte(op2);
}

void Compiler::emitReturn() { emitByte(OpCode::RETURN); }

void Compiler::emitConstant(Value value) {
  emitBytes(OpCode::CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
  auto constantIndex = currentChunk()->AddConstant(value);
  if (constantIndex >= UINT8_MAX) {
    parser_->error("Too many constants in one chunk.");
    return 0;
  }
  return static_cast<uint8_t>(constantIndex);
}

void Compiler::parsePrecedence(Compiler *compiler, Precedence precedence) {
  compiler->parser_->advance();
  ParseRule::ParseFn prefixRule =
      compiler->getRule(compiler->parser_->previous().type)->prefix;
  if (!prefixRule) {
    compiler->parser_->error("Expect expression.");
    return;
  }

  bool canAssign = precedence <= Precedence::ASSIGNMENT;
  prefixRule(compiler, canAssign);

  while (precedence <=
         compiler->getRule(compiler->parser_->current().type)->precedence) {
    compiler->parser_->advance();
    ParseRule::ParseFn infixRule =
        compiler->getRule(compiler->parser_->previous().type)->infix;
    if (!infixRule) {
      compiler->parser_->error("Expect expression.");
      return;
    }

    infixRule(compiler, canAssign);
  }

  if (canAssign && compiler->parser_->match(TokenType::EQUAL)) {
    compiler->parser_->error("Invalid assignment target.");
  }
}

const ParseRule *Compiler::getRule(TokenType type) {
  static std::unordered_map<TokenType, ParseRule> rules = {
      {TokenType::LEFT_PAREN, {Compiler::grouping, nullptr, Precedence::NONE}},
      {TokenType::RIGHT_PAREN, {nullptr, nullptr, Precedence::NONE}},
      {TokenType::MINUS, {Compiler::unary, Compiler::binary, Precedence::TERM}},
      {TokenType::PLUS, {nullptr, Compiler::binary, Precedence::TERM}},
      {TokenType::STAR, {nullptr, Compiler::binary, Precedence::FACTOR}},
      {TokenType::SLASH, {nullptr, Compiler::binary, Precedence::FACTOR}},
      {TokenType::NUMBER, {Compiler::number, nullptr, Precedence::NONE}},
      {TokenType::FALSE, {Compiler::literal, nullptr, Precedence::NONE}},
      {TokenType::TRUE, {Compiler::literal, nullptr, Precedence::NONE}},
      {TokenType::NIL, {Compiler::literal, nullptr, Precedence::NONE}},
      {TokenType::END_OF_FILE, {nullptr, nullptr, Precedence::NONE}},
      {TokenType::BANG, {Compiler::unary, nullptr, Precedence::NONE}},
      {TokenType::BANG_EQUAL,
       {nullptr, Compiler::binary, Precedence::EQUALITY}},
      {TokenType::EQUAL_EQUAL,
       {nullptr, Compiler::binary, Precedence::EQUALITY}},
      {TokenType::GREATER, {nullptr, Compiler::binary, Precedence::COMPARISON}},
      {TokenType::GREATER_EQUAL,
       {nullptr, Compiler::binary, Precedence::COMPARISON}},
      {TokenType::LESS, {nullptr, Compiler::binary, Precedence::COMPARISON}},
      {TokenType::LESS_EQUAL,
       {nullptr, Compiler::binary, Precedence::COMPARISON}},
      {TokenType::STRING, {Compiler::string, nullptr, Precedence::NONE}},
      {TokenType::IDENTIFIER, {Compiler::variable, nullptr, Precedence::NONE}},
      {TokenType::AND, {Compiler::logicalAnd, nullptr, Precedence::AND}},
      {TokenType::OR, {Compiler::logicalOr, nullptr, Precedence::OR}},
  };
  if (!rules.contains(type)) {
    throw std::runtime_error("No rule for token type: " +
                             std::to_string(static_cast<int>(type)));
  }
  return &rules[type];
}

void Compiler::expression(Compiler *compiler) {
  parsePrecedence(compiler, Precedence::ASSIGNMENT);
}

void Compiler::grouping(Compiler *compiler, bool canAssign) {
  expression(compiler);
  compiler->parser_->consume(TokenType::RIGHT_PAREN,
                             "Expect ')' after expression.");
}

void Compiler::unary(Compiler *compiler, bool canAssign) {
  TokenType operatorType = compiler->parser_->previous().type;

  expression(compiler);

  switch (operatorType) {
  case TokenType::MINUS:
    compiler->emitByte(OpCode::NEGATE);
    break;
  case TokenType::BANG:
    compiler->emitByte(OpCode::NOT);
    break;
  default:
    return;
  }
}

void Compiler::binary(Compiler *compiler, bool canAssign) {
  TokenType operatorType = compiler->parser_->previous().type;
  auto rule = getRule(operatorType);
  parsePrecedence(compiler, static_cast<Precedence>(
                                static_cast<int>(rule->precedence) + 1));

  switch (operatorType) {
  case TokenType::PLUS:
    compiler->emitByte(OpCode::ADD);
    break;
  case TokenType::MINUS:
    compiler->emitByte(OpCode::SUBTRACT);
    break;
  case TokenType::STAR:
    compiler->emitByte(OpCode::MULTIPLY);
    break;
  case TokenType::SLASH:
    compiler->emitByte(OpCode::DIVIDE);
    break;
  case TokenType::BANG_EQUAL:
    compiler->emitBytes(OpCode::EQUAL, OpCode::NOT);
    break;
  case TokenType::EQUAL_EQUAL:
    compiler->emitBytes(OpCode::EQUAL, OpCode::EQUAL);
    break;
  case TokenType::GREATER:
    compiler->emitByte(OpCode::GREATER);
    break;
  case TokenType::GREATER_EQUAL:
    compiler->emitBytes(OpCode::LESS, OpCode::NOT);
    break;
  case TokenType::LESS:
    compiler->emitByte(OpCode::LESS);
    break;
  case TokenType::LESS_EQUAL:
    compiler->emitBytes(OpCode::GREATER, OpCode::NOT);
  default:
    return;
  }
}

void Compiler::number(Compiler *compiler, bool canAssign) {
  double value = std::stod(compiler->parser_->previous().start);
  compiler->emitConstant(Value::Number(value));
}

void Compiler::string(Compiler *compiler, bool canAssign) {
  compiler->emitConstant(Value::Object(
      ObjString::getObject(compiler->parser_->previous().start + 1,
                           compiler->parser_->previous().length - 2)));
}

void Compiler::literal(Compiler *compiler, bool canAssign) {
  switch (compiler->parser_->previous().type) {
  case TokenType::FALSE:
    compiler->emitByte(OpCode::FALSE);
    break;
  case TokenType::TRUE:
    compiler->emitByte(OpCode::TRUE);
    break;
  case TokenType::NIL:
    compiler->emitByte(OpCode::NIL);
    break;
  default:
    return;
  }
}

void Compiler::declaration(Compiler *compiler) {
  if (compiler->parser_->match(TokenType::VAR)) {
    varDeclaration(compiler);
  } else {
    statement(compiler);
  }
  if (compiler->parser_->panicMode()) {
    synchronize(compiler);
  }
}

void Compiler::varDeclaration(Compiler *compiler) {
  auto global = parseVariable(compiler, "Expect variable name.");

  if (compiler->parser_->match(TokenType::EQUAL)) {
    expression(compiler);
  } else {
    compiler->emitByte(OpCode::NIL);
  }
  compiler->parser_->consume(TokenType::SEMICOLON, "Expect ';' after value.");
  defineVariable(compiler, global);
}

uint8_t Compiler::parseVariable(Compiler *compiler,
                                const std::string &errorMessage) {
  compiler->parser_->consume(TokenType::IDENTIFIER, errorMessage);
  declareVariable(compiler);
  if (compiler->scopeDepth_ > 0) {
    return 0;
  }
  return compiler->identifierConstant(compiler->parser_->previous());
}

void Compiler::declareVariable(Compiler *compiler) {
  if (compiler->scopeDepth_ == 0) {
    return;
  }
  const auto &name = compiler->parser_->previous();
  // check if the variable is already declared in this scope
  for (int i = compiler->locals_.size() - 1; i >= 0; i--) {
    auto &local = compiler->locals_[i];
    if (local.depth != -1 && local.depth < compiler->scopeDepth_) {
      break;
    }
    if (identifiersEqual(local.name, name)) {
      compiler->parser_->error(
          "Variable with this name already declared in this scope.");
    }
  }
  compiler->addLocal(name);
}

uint8_t Compiler::identifierConstant(const Token &name) {
  return makeConstant(
      Value::Object(ObjString::getObject(name.start, name.length)));
}

void Compiler::defineVariable(Compiler *compiler, uint8_t global) {
  if (compiler->scopeDepth_ > 0) {
    compiler->markInitialized();
    return;
  }
  compiler->emitBytes(OpCode::DEFINE_GLOBAL, global);
}

void Compiler::addLocal(const Token &name) {
  if (locals_.size() == UINT8_MAX) {
    parser_->error("Too many local variables in function.");
    return;
  }
  locals_.push_back({name, -1 /* set depth -1 to indicate not initialized */});
}

void Compiler::markInitialized() { locals_.back().depth = scopeDepth_; }

void Compiler::statement(Compiler *compiler) {
  if (compiler->parser_->match(TokenType::PRINT)) {
    printStatement(compiler);
  } else if (compiler->parser_->match(TokenType::IF)) {
    ifStatement(compiler);
  } else if (compiler->parser_->match(TokenType::WHILE)) {
    whileStatement(compiler);
  } else if (compiler->parser_->match(TokenType::FOR)) {
    forStatement(compiler);
  } else if (compiler->parser_->match(TokenType::LEFT_BRACE)) {
    block(compiler);
  } else {
    expressionStatement(compiler);
  }
}

void Compiler::forStatement(Compiler *compiler) {
  beginScope(compiler);

  compiler->parser_->consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
  if (compiler->parser_->match(TokenType::SEMICOLON)) {
    // empty initializer
  } else if (compiler->parser_->match(TokenType::VAR)) {
    varDeclaration(compiler);
  } else {
    expressionStatement(compiler);
  }

  int loopStart = compiler->currentChunk()->code.size();
  int exitJump = -1;
  if (!compiler->parser_->match(TokenType::SEMICOLON)) {
    expression(compiler);
    compiler->parser_->consume(TokenType::SEMICOLON,
                               "Expect ';' after for loop condition.");
    // Jump to the end of the loop if the condition is false
    exitJump = compiler->emitJump(OpCode::JUMP_IF_FALSE);
    compiler->emitByte(OpCode::POP);
  }

  if (compiler->parser_->match(TokenType::RIGHT_PAREN)) {
    int bodyJump = compiler->emitJump(OpCode::JUMP);
    int incrementStart = compiler->currentChunk()->code.size();
    expression(compiler);
    compiler->emitByte(OpCode::POP);
    compiler->parser_->consume(TokenType::RIGHT_PAREN,
                               "Expect ')' after for clauses.");

    compiler->emitLoop(loopStart);
    loopStart = incrementStart;
    compiler->patchJump(bodyJump);
  }

  compiler->statement(compiler);
  compiler->emitLoop(loopStart);

  if (exitJump != -1) {
    compiler->patchJump(exitJump);
    compiler->emitByte(OpCode::POP);
  }

  endScope(compiler);
}

void Compiler::whileStatement(Compiler *compiler) {
  int loopStart = compiler->currentChunk()->code.size();

  compiler->parser_->consume(TokenType::LEFT_PAREN,
                             "Expect '(' after 'while'.");
  expression(compiler);
  compiler->parser_->consume(TokenType::RIGHT_PAREN,
                             "Expect ')' after condition.");

  int exitJump = compiler->emitJump(OpCode::JUMP_IF_FALSE);
  compiler->emitByte(OpCode::POP);

  compiler->statement(compiler);

  compiler->emitLoop(loopStart);

  compiler->patchJump(exitJump);
  compiler->emitByte(OpCode::POP);
}

void Compiler::emitLoop(int loopStart) {
  emitByte(OpCode::LOOP);
  int offset = currentChunk()->code.size() - loopStart + 2;
  if (offset > UINT16_MAX) {
    parser_->error("Loop body too large.");
  }
  emitByte((offset >> 8) & 0xFF);
  emitByte(offset & 0xFF);
}

void Compiler::ifStatement(Compiler *compiler) {
  compiler->parser_->consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
  expression(compiler);
  compiler->parser_->consume(TokenType::RIGHT_PAREN,
                             "Expect ')' after condition.");

  int thenJump = compiler->emitJump(OpCode::JUMP_IF_FALSE);
  compiler->emitByte(OpCode::POP);
  compiler->statement(compiler);

  int elseJump = compiler->emitJump(OpCode::JUMP);

  compiler->patchJump(thenJump);

  compiler->emitByte(OpCode::POP);
  if (compiler->parser_->match(TokenType::ELSE)) {
    compiler->statement(compiler);
  }

  compiler->patchJump(elseJump);
}

int Compiler::emitJump(OpCode op) {
  emitByte(op);
  emitByte(0xFF);
  emitByte(0xFF);
  return currentChunk()->code.size() - 2;
}

void Compiler::patchJump(int offset) {
  // -2 to adjust for the bytecode for the jump offset itself.
  int jump = currentChunk()->code.size() - offset - 2;
  if (jump > UINT16_MAX) {
    parser_->error("Too much code to jump over.");
  }

  auto &code = currentChunk()->code;
  code[offset] = (jump >> 8) & 0xFF;
  code[offset + 1] = jump & 0xFF;
}

void Compiler::logicalAnd(Compiler *compiler, bool canAssign) {
  int endJump = compiler->emitJump(OpCode::JUMP_IF_FALSE);

  compiler->emitByte(OpCode::POP);
  compiler->parsePrecedence(compiler, Precedence::AND);

  compiler->patchJump(endJump);
}

void Compiler::logicalOr(Compiler *compiler, bool canAssign) {
  int elseJump = compiler->emitJump(OpCode::JUMP_IF_FALSE);
  int endJump = compiler->emitJump(OpCode::JUMP);

  compiler->patchJump(elseJump);
  compiler->emitByte(OpCode::POP);

  compiler->parsePrecedence(compiler, Precedence::OR);
  compiler->patchJump(endJump);
}

void Compiler::expressionStatement(Compiler *compiler) {
  expression(compiler);
  compiler->parser_->consume(TokenType::SEMICOLON, "Expect ';' after value.");
  compiler->emitByte(OpCode::POP);
}

void Compiler::printStatement(Compiler *compiler) {
  expression(compiler);
  compiler->parser_->consume(TokenType::SEMICOLON, "Expect ';' after value.");
  compiler->emitByte(OpCode::PRINT);
}

void Compiler::synchronize(Compiler *compiler) {
  compiler->parser_->resetPanicMode();

  while (compiler->parser_->current().type != TokenType::END_OF_FILE) {
    if (compiler->parser_->previous().type == TokenType::SEMICOLON) {
      return;
    }
    switch (compiler->parser_->current().type) {
    case TokenType::CLASS:
    case TokenType::FUN:
    case TokenType::VAR:
    case TokenType::FOR:
    case TokenType::IF:
    case TokenType::WHILE:
    case TokenType::PRINT:
    case TokenType::RETURN:
      return;
    default:; // do nothing
    }
    compiler->parser_->advance();
  }
}

void Compiler::variable(Compiler *compiler, bool canAssign) {
  namedVariable(compiler, compiler->parser_->previous(), canAssign);
}

int Compiler::resolveLocal(const Token &name) {
  for (int i = locals_.size() - 1; i >= 0; i--) {
    auto &local = locals_[i];
    if (identifiersEqual(local.name, name)) {
      if (local.depth == -1) {
        parser_->error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }

  return -1;
}

void Compiler::namedVariable(Compiler *compiler, const Token &name,
                             bool canAssign) {

  OpCode getOp, setOp;
  int arg = compiler->resolveLocal(name);
  if (arg != -1) {
    getOp = OpCode::GET_LOCAL;
    setOp = OpCode::SET_LOCAL;
  } else {
    arg = compiler->identifierConstant(name);
    getOp = OpCode::GET_GLOBAL;
    setOp = OpCode::SET_GLOBAL;
  }

  if (canAssign && compiler->parser_->match(TokenType::EQUAL)) {
    expression(compiler);
    compiler->emitBytes(setOp, static_cast<uint8_t>(arg));
  } else {
    compiler->emitBytes(getOp, static_cast<uint8_t>(arg));
  }
}

void Compiler::block(Compiler *compiler) {
  while (!compiler->parser_->check(TokenType::RIGHT_BRACE) &&
         !compiler->parser_->check(TokenType::END_OF_FILE)) {
    declaration(compiler);
  }
  compiler->parser_->consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::beginScope(Compiler *compiler) { compiler->scopeDepth_++; }

void Compiler::endScope(Compiler *compiler) {
  compiler->scopeDepth_--;
  while (compiler->locals_.size() > 0 &&
         compiler->locals_.back().depth > compiler->scopeDepth_) {
    compiler->emitByte(OpCode::POP);
    compiler->locals_.pop_back();
  }
}