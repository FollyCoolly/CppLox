#include "compiler.h"
#include "scanner.h"
#include <iostream>

void compile(const std::string &source) {
  int line = -1;
  Scanner scanner(source);
  while (true) {
    Token token = scanner.scanToken();
    if (token.line != line) {
      line = token.line;
      std::cout << std::format("{:4} ", line);
    } else {
      std::cout << "   | ";
    }
    std::cout << std::string_view(token.start, token.length) << " ";

    if (token.type == TokenType::END_OF_FILE) {
      break;
    }
  }
}
