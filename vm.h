#pragma once

#include "chunk.h"
#include "value.h"
#include <memory>
#include <unordered_map>
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
  std::unordered_map<std::string, Value> globals_;

  VM() = default;
  ~VM() = default;

  std::shared_ptr<Chunk> chunk_;
  int codeIdx_;
  std::vector<Value> stack_;

  InterpretResult run();

  Value pop();
  void push(Value value);
  Value peek(int distance) const;
  void printStack();
  void resetStack();

  void runtimeError(const std::string &message);

  static bool isFalsey(const Value &value);
};
