#include "vm.h"
#include "chunk.h"
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
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if (!Value::IsNumber(peek(0)) || !Value::IsNumber(peek(1))) {              \
      runtimeError("Operands must be numbers.");                               \
      return InterpretResult::InterpretRuntimeError;                           \
    }                                                                          \
    double b = Value::AsNumber(pop());                                         \
    double a = Value::AsNumber(pop());                                         \
    push(valueType(a op b));                                                   \
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
      if (!Value::IsNumber(peek(0))) {
        runtimeError("Operand must be a number.");
        return InterpretResult::InterpretRuntimeError;
      }
      push(Value::Number(-Value::AsNumber(pop())));
      break;
    case OpCode::ADD:
      BINARY_OP(Value::Number, +);
      break;
    case OpCode::SUBTRACT:
      BINARY_OP(Value::Number, -);
      break;
    case OpCode::MULTIPLY:
      BINARY_OP(Value::Number, *);
      break;
    case OpCode::DIVIDE:
      BINARY_OP(Value::Number, /);
      break;
    case OpCode::FALSE:
      push(Value::Bool(false));
      break;
    case OpCode::TRUE:
      push(Value::Bool(true));
      break;
    case OpCode::NIL:
      push(Value::Nil());
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

Value VM::peek(int distance) const {
  return stack_[stack_.size() - 1 - distance];
}

void VM::printStack() {
  for (auto &value : stack_) {
    std::cout << std::format("[ {} ] ", value);
  }
  std::cout << std::endl;
}

void VM::resetStack() { stack_.clear(); }

void VM::runtimeError(const std::string &message) {
  std::cerr << message << std::endl;

  size_t instruction = codeIdx_ - 1;
  int line = chunk_->lines[instruction];
  std::cerr << "[line " << line << "] in script" << std::endl;

  resetStack();
}