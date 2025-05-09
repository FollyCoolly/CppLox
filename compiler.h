#pragma once

#include "chunk.h"
#include <string>
class Compiler {
public:
  std::shared_ptr<Chunk> compile(const std::string &source);

private:
  std::shared_ptr<Chunk> compilingChunk_;
};