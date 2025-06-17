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

std::shared_ptr<Chunk> Compiler::compile(const std::string &source) {
  parser_ = std::make_unique<Parser>(source);
  compilingChunk_ = std::make_shared<Chunk>();
  parser_->advance();
  while (!parser_->match(TokenType::END_OF_FILE)) {
    declaration(this);
  }
  endCompiler();
  return compilingChunk_;
}

void Compiler::endCompiler() {
  emitReturn();
#ifdef DEBUG_PRINT_CODE
  disassembleChunk(*compilingChunk_, "code");
#endif
}

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

void Compiler::emitBytes(OpCode op1, OpCode op2) {
  emitByte(op1);
  emitByte(op2);
}

void Compiler::emitReturn() { emitByte(OpCode::RETURN); }

void Compiler::emitConstant(Value value) {
  emitBytes(OpCode::CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
  auto constantIndex = compilingChunk_->AddConstant(value);
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

  prefixRule(compiler);

  while (precedence <=
         compiler->getRule(compiler->parser_->current().type)->precedence) {
    compiler->parser_->advance();
    ParseRule::ParseFn infixRule =
        compiler->getRule(compiler->parser_->previous().type)->infix;
    if (!infixRule) {
      compiler->parser_->error("Expect expression.");
      return;
    }

    infixRule(compiler);
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

void Compiler::grouping(Compiler *compiler) {
  expression(compiler);
  compiler->parser_->consume(TokenType::RIGHT_PAREN,
                             "Expect ')' after expression.");
}

void Compiler::unary(Compiler *compiler) {
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

void Compiler::binary(Compiler *compiler) {
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

void Compiler::number(Compiler *compiler) {
  double value = std::stod(compiler->parser_->previous().start);
  compiler->emitConstant(Value::Number(value));
}

void Compiler::string(Compiler *compiler) {
  compiler->emitConstant(Value::Object(
      ObjString::getObject(compiler->parser_->previous().start + 1,
                           compiler->parser_->previous().length - 2)));
}

void Compiler::literal(Compiler *compiler) {
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
  return compiler->identifierConstant(compiler->parser_->previous().start);
}

uint8_t Compiler::identifierConstant(Compiler *compiler, const Token &name) {
  return compiler->makeConstant(
      Value::Object(ObjString::getObject(name.start, name.length)));
}

void Compiler::defineVariable(Compiler *compiler, uint8_t global) {
  compiler->emitBytes(OpCode::DEFINE_GLOBAL, global);
}

void Compiler::statement(Compiler *compiler) {
  if (compiler->parser_->match(TokenType::PRINT)) {
    printStatement(compiler);
  } else {
    expressionStatement(compiler);
  }
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