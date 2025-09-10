#pragma once

#include "chunk.h"
#include "object.h"
#include "value.h"
#include <cstddef>
#include <forward_list>
#include <memory>
#include <unordered_map>
#include <vector>

enum class InterpretResult {
  InterpretOk,
  InterpretCompileError,
  InterpretRuntimeError,
};

struct CallFrame {
  ObjClosure *closure;
  int codeIdx;
  size_t valueIdx;
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
  std::forward_list<std::shared_ptr<ObjUpvalue>> openUpvalues_;

  InterpretResult run();

  Value pop();
  void push(Value value);
  Value peek(int distance) const;
  void printStack();
  void resetStack();
  bool callValue(Value callee, uint8_t argCount);
  bool call(ObjClosure *closure, uint8_t argCount);

  void runtimeError(const std::string &message);
  void defineNative(const std::string &name, NativeFunction function);
  std::shared_ptr<ObjUpvalue> captureUpvalue(uint8_t index);

  static bool isFalsey(const Value &value);
};
