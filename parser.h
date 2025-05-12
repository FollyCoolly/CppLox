#pragma once

#include "scanner.h"

class Parser {
public:
  Parser(const std::string &source);

  void advance();
  void consume(TokenType type, const std::string &message);

  void errorAtCurrent(const std::string &message);
  void error(const std::string &message);
  void errorAt(const Token &token, const std::string &message);

  bool hadError() const { return hadError_; }

  const Token &previous() const { return previous_; }
  const Token &current() const { return current_; }

private:
  Scanner scanner_;
  Token current_;
  Token previous_;
  bool hadError_{false};
  bool panicMode_{false};
};
