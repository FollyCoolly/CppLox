#pragma once

#include "chunk.h"
#include "object.h"
#include "value.h"
#include <memory>
#include <unordered_map>
#include <vector>

enum class InterpretResult {
  InterpretOk,
  InterpretCompileError,
  InterpretRuntimeError,
};

struct CallFrame {
  std::shared_ptr<ObjFunction> function;
  int codeIdx;
  Value *slots;
};

class VM {
public:
  static constexpr int FRAMES_MAX = 64;
  static constexpr int STACK_MAX =
      FRAMES_MAX * 256; // 8 bits can represent 256 values

  InterpretResult interpret(const std::string &source);

private:
  std::unordered_map<std::string, Value> globals_;

  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;

  InterpretResult run();

  Value pop();
  void push(Value value);
  Value peek(int distance) const;
  void printStack();
  void resetStack();

  void runtimeError(const std::string &message);

  static bool isFalsey(const Value &value);
};
