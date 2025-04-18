#pragma once

#include "chunk.h"
#include <memory>

enum class InterpretResult {
  InterpretOk,
  InterpretCompileError,
  InterpretRuntimeError,
};

class VM {
public:
  static VM &getInstance();

  InterpretResult interpret(std::shared_ptr<Chunk> chunk);

private:
  VM() = default;
  ~VM() = default;

  std::shared_ptr<Chunk> chunk_;
  int codeIdx_;

  InterpretResult run();
};
