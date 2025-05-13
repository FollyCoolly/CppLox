#include "vm.h"
#include "compiler.h"
#include "debug.h"
#include <iostream>

VM &VM::getInstance() {
  static VM instance;
  return instance;
}

InterpretResult VM::interpret(const std::string &source) {
  Compiler compiler;
  auto chunk = compiler.compile(source);
  return getInstance().interpret(chunk);
}

InterpretResult VM::interpret(std::shared_ptr<Chunk> chunk) {
  chunk_ = std::move(chunk);
  codeIdx_ = 0;
  return run();
}

InterpretResult VM::run() {
  auto readByte = [this]() -> uint8_t { return chunk_->code[codeIdx_++]; };
  auto readConstant = [this, &readByte]() -> Value {
    return chunk_->constants[readByte()];
  };
#define BINARY_OP(op)                                                          \
  do {                                                                         \
    Value right = pop();                                                       \
    Value left = pop();                                                        \
    push(left op right);                                                       \
  } while (false)

  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    disassembleInstruction(*chunk_, codeIdx_);
    printStack();
#endif
    uint8_t instruction = readByte();
    switch (from_uint8(instruction)) {
    case OpCode::RETURN:
      std::cout << pop() << std::endl;
      return InterpretResult::InterpretOk;
    case OpCode::NEGATE:
      push(-pop());
      break;
    case OpCode::ADD:
      BINARY_OP(+);
      break;
    case OpCode::SUBTRACT:
      BINARY_OP(-);
      break;
    case OpCode::MULTIPLY:
      BINARY_OP(*);
      break;
    case OpCode::DIVIDE:
      BINARY_OP(/);
      break;
    case OpCode::CONSTANT:
      Value constant = readConstant();
      push(constant);
      break;
    }
  }
#undef BINARY_OP
}

Value VM::pop() {
  Value value = stack_.back();
  stack_.pop_back();
  return value;
}

void VM::push(Value value) { stack_.push_back(value); }

void VM::printStack() {
  for (auto &value : stack_) {
    std::cout << std::format("[ {} ] ", value);
  }
  std::cout << std::endl;
}