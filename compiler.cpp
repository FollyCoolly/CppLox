#include "compiler.h"
#include "parser.h"
#include "scanner.h"
#include <iostream>

std::shared_ptr<Chunk> Compiler::compile(const std::string &source) {
  Parser parser(source);

  parser.advance();
}
