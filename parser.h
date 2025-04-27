#pragma once

#include "scanner.h"

class Parser {
public:
  Parser(const std::string &source);

  void advance();
  void consume(TokenType type, const std::string &message);

private:
  Scanner scanner_;
  Token current_;
  Token previous_;
};
