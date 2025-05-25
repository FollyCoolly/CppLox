#include "compiler.h"
#include "parser.h"
#include "scanner.h"
#include <cstdint>
#include <unordered_map>
#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

std::shared_ptr<Chunk> Compiler::compile(const std::string &source) {
  parser_ = std::make_unique<Parser>(source);
  compilingChunk_ = std::make_shared<Chunk>();
  parser_->advance();
  expression(this);
  parser_->consume(TokenType::END_OF_FILE, "Expect end of expression.");
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
  default:
    return;
  }
}

void Compiler::number(Compiler *compiler) {
  double value = std::stod(compiler->parser_->previous().start);
  compiler->emitConstant(Value::Number(value));
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
