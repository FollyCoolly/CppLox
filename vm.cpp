#include "vm.h"
#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include <iostream>
#include <ranges>

InterpretResult VM::interpret(const std::string &source) {
  Compiler compiler;
  auto function = compiler.compile(source);
  if (function == nullptr) {
    return InterpretResult::InterpretCompileError;
  }

  push(Value::Object(function.get()));
  call(function.get(), 0);

  return run();
}

InterpretResult VM::run() {
  auto &currentFrame = frames_.back();
  auto readByte = [this, &currentFrame]() -> uint8_t {
    return currentFrame.function->chunk->code[currentFrame.codeIdx++];
  };
  auto readConstant = [this, &readByte, &currentFrame]() -> Value {
    return currentFrame.function->chunk->constants[readByte()];
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
    disassembleInstruction(*currentFrame.function->chunk, currentFrame.codeIdx);
    printStack();
#endif
    uint8_t instruction = readByte();
    switch (from_uint8(instruction)) {
    case OpCode::RETURN: {
      frames_.pop_back();
      if (frames_.empty()) {
        pop();
        return InterpretResult::InterpretOk;
      }
      break;
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
        currentFrame.codeIdx += offset;
      }
      break;
    }
    case OpCode::JUMP: {
      uint16_t offset = readByte() << 8 | readByte();
      currentFrame.codeIdx += offset;
      break;
    }
    case OpCode::LOOP: {
      uint16_t offset = readByte() << 8 | readByte();
      currentFrame.codeIdx -= offset;
      break;
    }
    case OpCode::CALL: {
      uint8_t argCount = readByte();
      if (!callValue(peek(argCount), argCount)) {
        return InterpretResult::InterpretRuntimeError;
      }
      break;
    }
    }
  }
#undef BINARY_OP
}

bool VM::callValue(Value callee, uint8_t argCount) {
  if (obj_helpers::IsObjType(callee, Obj::Type::FUNCTION)) {
    return call(obj_helpers::AsFunction(callee), argCount);
  }

  runtimeError("Can only call functions.");
  return false;
}

bool VM::call(ObjFunction *function, uint8_t argCount) {
  if (argCount != function->arity) {
    runtimeError("Expected " + std::to_string(function->arity) +
                 " arguments but got " + std::to_string(argCount) + ".");
    return false;
  }

  if (frames_.size() + 1 > FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  frames_.emplace_back(CallFrame{function, 0, stack_.size() - argCount - 1});
  return true;
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

  for (const auto &frame : std::views::reverse(frames_)) {
    auto function = frame.function;
    auto codeIdx = frame.codeIdx;
    auto chunk = function->chunk;
    auto line = chunk->lines[codeIdx];
    auto name = function->name->str.empty() ? function->name->str : "script";
    std::cerr << "[line " << line << "] in " << name << std::endl;
  }

  resetStack();
}

bool VM::isFalsey(const Value &value) {
  return Value::IsNil(value) || (Value::IsBool(value) && !Value::AsBool(value));
}