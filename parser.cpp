#include "parser.h"

Parser::Parser(const std::string &source) : scanner_(source) {}

void Parser::advance() {
  previous_ = current_;
  current_ = scanner_.scanToken();
}

void Parser::consume(TokenType type, const std::string &message) {
  if (current_.type == type) {
    advance();
  }
}
