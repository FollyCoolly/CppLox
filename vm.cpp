#include "vm.h"
#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include <iostream>

VM &VM::getInstance() {
  static VM instance;
  return instance;
}

InterpretResult VM::interpret(const std::string &source) {
  Compiler compiler;
  auto function = compiler.compile(source);
  return getInstance().interpret(function->chunk);
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
  auto readString = [this, &readConstant]() -> std::string {
    return obj_helpers::AsString(readConstant())->str;
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
    case OpCode::RETURN: {
      std::cout << pop() << std::endl;
      return InterpretResult::InterpretOk;
    }
    case OpCode::NEGATE: {
      if (!Value::IsNumber(peek(0))) {
        runtimeError("Operand must be a number.");
        return InterpretResult::InterpretRuntimeError;
      }
      push(Value::Number(-Value::AsNumber(pop())));
      break;
    }
    case OpCode::ADD: {
      if (obj_helpers::IsString(peek(0)) && obj_helpers::IsString(peek(1))) {
        const auto &a = obj_helpers::AsString(peek(0))->str;
        const auto &b = obj_helpers::AsString(peek(1))->str;
        push(Value::Object(ObjString::getObject(a + b)));
      } else if (Value::IsNumber(peek(0)) && Value::IsNumber(peek(1))) {
        double b = Value::AsNumber(pop());
        double a = Value::AsNumber(pop());
        push(Value::Number(a + b));
      } else {
        runtimeError("Operands must be numbers or strings.");
        return InterpretResult::InterpretRuntimeError;
      }
      break;
    }
    case OpCode::SUBTRACT: {
      BINARY_OP(Value::Number, -);
      break;
    }
    case OpCode::MULTIPLY: {
      BINARY_OP(Value::Number, *);
      break;
    }
    case OpCode::DIVIDE: {
      BINARY_OP(Value::Number, /);
      break;
    }
    case OpCode::NOT: {
      push(Value::Bool(!isFalsey(peek(0))));
      break;
    }
    case OpCode::FALSE: {
      push(Value::Bool(false));
      break;
    }
    case OpCode::TRUE: {
      push(Value::Bool(true));
      break;
    }
    case OpCode::NIL: {
      push(Value::Nil());
      break;
    }
    case OpCode::CONSTANT: {
      Value constant = readConstant();
      push(constant);
      break;
    }
    case OpCode::EQUAL: {
      Value b = pop();
      Value a = pop();
      push(Value::Bool(a == b));
      break;
    }
    case OpCode::GREATER: {
      BINARY_OP(Value::Bool, >);
      break;
    }
    case OpCode::LESS: {
      BINARY_OP(Value::Bool, <);
      break;
    }
    case OpCode::PRINT: {
      std::cout << pop() << std::endl;
      break;
    }
    case OpCode::POP: {
      pop();
      break;
    }
    case OpCode::DEFINE_GLOBAL: {
      std::string name = readString();
      globals_[name] = pop();
      break;
    }
    case OpCode::GET_GLOBAL: {
      std::string name = readString();
      auto it = globals_.find(name);
      if (it == globals_.end()) {
        runtimeError("Undefined variable '" + name + "'.");
        return InterpretResult::InterpretRuntimeError;
      }
      push(it->second);
      break;
    }
    case OpCode::SET_GLOBAL: {
      std::string name = readString();
      auto it = globals_.find(name);
      if (it == globals_.end()) {
        runtimeError("Undefined variable '" + name + "'.");
        return InterpretResult::InterpretRuntimeError;
      }
      globals_[name] = peek(0);
      break;
    }
    case OpCode::GET_LOCAL: {
      uint8_t slot = readByte();
      push(stack_[slot]);
      break;
    }
    case OpCode::SET_LOCAL: {
      uint8_t slot = readByte();
      stack_[slot] = peek(0);
      break;
    }
    case OpCode::JUMP_IF_FALSE: {
      uint16_t offset = readByte() << 8 | readByte();
      if (isFalsey(peek(0))) {
        codeIdx_ += offset;
      }
      break;
    }
    case OpCode::JUMP: {
      uint16_t offset = readByte() << 8 | readByte();
      codeIdx_ += offset;
      break;
    }
    case OpCode::LOOP: {
      uint16_t offset = readByte() << 8 | readByte();
      codeIdx_ -= offset;
      break;
    }
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
    std::cout << std::vformat("[ {} ] ", std::make_format_args(value));
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

bool VM::isFalsey(const Value &value) {
  return Value::IsNil(value) || (Value::IsBool(value) && !Value::AsBool(value));
}