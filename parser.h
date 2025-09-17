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

  bool hadError() const { return had_error_; }

  const Token &previous() const { return previous_; }
  const Token &current() const { return current_; }

  bool match(TokenType type);
  bool check(TokenType type) const;

  bool panicMode() const { return panic_mode_; }
  void resetPanicMode() { panic_mode_ = false; }

private:
  Scanner scanner_;
  Token current_;
  Token previous_;
  bool had_error_{false};
  bool panic_mode_{false};
};
