#pragma once

#include "chunk.h"
#include <memory>
#include <vector>

enum class InterpretResult {
  InterpretOk,
  InterpretCompileError,
  InterpretRuntimeError,
};

class VM {
public:
  static VM &getInstance();
  static InterpretResult interpret(const std::string &source);

  InterpretResult interpret(std::shared_ptr<Chunk> chunk);

private:
  VM() = default;
  ~VM() = default;

  std::shared_ptr<Chunk> chunk_;
  int codeIdx_;
  std::vector<Value> stack_;

  InterpretResult run();

  Value pop();
  void push(Value value);
  void printStack();
};
