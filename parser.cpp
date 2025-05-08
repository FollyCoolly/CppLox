#include "parser.h"
#include <iostream>
#include <string_view>
Parser::Parser(const std::string &source) : scanner_(source) {}

void Parser::advance() {
  previous_ = current_;
  while (true) {
    current_ = scanner_.scanToken();
    if (current_.type == TokenType::ERROR) {
      hadError_ = true;
    }
    break;
  }
}

void Parser::consume(TokenType type, const std::string &message) {
  if (current_.type == type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

void Parser::errorAtCurrent(const std::string &message) {
  errorAt(current_, message);
}

void Parser::error(const std::string &message) { errorAt(previous_, message); }

void Parser::errorAt(const Token &token, const std::string &message) {
  if (panicMode_) {
    return;
  }

  panicMode_ = true;
  std::cerr << std::format("[line {}] Error", token.line);

  if (token.type == TokenType::END_OF_FILE) {
    std::cerr << " at end";
  } else if (token.type == TokenType::ERROR) {
    // nothing
  } else {
    std::cerr << std::format(" at '{}'",
                             std::string_view(token.start, token.length));
  }
  std::cerr << std::format(": {}", message) << std::endl;

  hadError_ = true;
}