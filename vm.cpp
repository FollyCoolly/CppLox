#include "vm.h"
#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include <cstdint>
#include <format>
#include <iostream>
#include <memory>
#include <ranges>

namespace {
Value clockNative(int arg_count, Value *args) {
  return Value::Number(
      std::chrono::system_clock::now().time_since_epoch().count());
}
} // namespace

InterpretResult VM::interpret(const std::string &source) {
  Compiler compiler;
  auto function = compiler.compile(source);
  if (function == nullptr) {
    return InterpretResult::InterpretCompileError;
  }

  auto closure = std::make_shared<ObjClosure>(function.get());
  push(Value::Object(closure));
  call(closure.get(), 0);

  return run();
}

InterpretResult VM::run() {
  auto &current_frame = frames_.back();
  auto read_byte = [this, &current_frame]() -> uint8_t {
    return current_frame.closure->function->chunk
        ->code[current_frame.code_idx++];
  };
  auto read_constant = [this, &read_byte, &current_frame]() -> Value {
    return current_frame.closure->function->chunk->constants[read_byte()];
  };
  auto read_string = [this, &read_constant]() -> std::string {
    return obj_helpers::AsString(read_constant())->str;
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
    disassembleInstruction(*current_frame.closure->function->chunk,
                           current_frame.code_idx);
    printStack();
#endif
    uint8_t instruction = read_byte();
    switch (from_uint8(instruction)) {
    case OpCode::RETURN: {
      closeUpvalues(frames_.back().value_idx);
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
      Value constant = read_constant();
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
      std::string name = read_string();
      globals_[name] = pop();
      break;
    }
    case OpCode::GET_GLOBAL: {
      std::string name = read_string();
      auto it = globals_.find(name);
      if (it == globals_.end()) {
        runtimeError("Undefined variable '" + name + "'.");
        return InterpretResult::InterpretRuntimeError;
      }
      push(it->second);
      break;
    }
    case OpCode::SET_GLOBAL: {
      std::string name = read_string();
      auto it = globals_.find(name);
      if (it == globals_.end()) {
        runtimeError("Undefined variable '" + name + "'.");
        return InterpretResult::InterpretRuntimeError;
      }
      globals_[name] = peek(0);
      break;
    }
    case OpCode::GET_LOCAL: {
      uint8_t slot = read_byte();
      push(stack_[slot]);
      break;
    }
    case OpCode::SET_LOCAL: {
      uint8_t slot = read_byte();
      stack_[slot] = peek(0);
      break;
    }
    case OpCode::JUMP_IF_FALSE: {
      uint16_t offset = read_byte() << 8 | read_byte();
      if (isFalsey(peek(0))) {
        current_frame.code_idx += offset;
      }
      break;
    }
    case OpCode::JUMP: {
      uint16_t offset = read_byte() << 8 | read_byte();
      current_frame.code_idx += offset;
      break;
    }
    case OpCode::LOOP: {
      uint16_t offset = read_byte() << 8 | read_byte();
      current_frame.code_idx -= offset;
      break;
    }
    case OpCode::CALL: {
      uint8_t arg_count = read_byte();
      if (!callValue(peek(arg_count), arg_count)) {
        return InterpretResult::InterpretRuntimeError;
      }
      break;
    }
    case OpCode::CLOSURE: {
      auto function = obj_helpers::AsFunction(read_constant());
      auto closure = std::make_shared<ObjClosure>(function);
      push(Value::Object(closure));
      for (int i = 0; i < closure->upvalue_count; i++) {
        auto isLocal = read_byte();
        auto index = read_byte();
        if (isLocal) {
          closure->upvalues[i] = captureUpvalue(index);
        } else {
          closure->upvalues[i] = current_frame.closure->upvalues[index];
        }
      }

      break;
    }
    case OpCode::GET_UPVALUE: {
      uint8_t slot = read_byte();
      push(stack_[current_frame.closure->upvalues[slot]->stack_idx]);
      break;
    }
    case OpCode::SET_UPVALUE: {
      uint8_t slot = read_byte();
      stack_[current_frame.closure->upvalues[slot]->stack_idx] = peek(0);
      break;
    }
    case OpCode::CLOSE_UPVALUE: {
      closeUpvalues(stack_.size() - 1);
      pop();
      break;
    }
    case OpCode::CLASS: {
      push(Value::Object(
          std::make_shared<ObjClass>(obj_helpers::AsString(read_constant()))));
      break;
    }
    case OpCode::GET_PROPERTY: {
      if (!obj_helpers::IsInstance(peek(0))) {
        runtimeError("Only instances have properties.");
        return InterpretResult::InterpretRuntimeError;
      }

      auto instance = obj_helpers::AsInstance(peek(0));
      auto name = read_string();
      if (instance->fields.contains(name)) {
        pop();
        push(instance->fields[name]);
        break;
      }

      if (!bindMethod(instance->klass, name)) {
        return InterpretResult::InterpretRuntimeError;
      }
      break;
    }
    case OpCode::SET_PROPERTY: {
      if (!obj_helpers::IsInstance(peek(1))) {
        runtimeError("Only instances have properties.");
        return InterpretResult::InterpretRuntimeError;
      }
      auto instance = obj_helpers::AsInstance(peek(1));
      instance->fields[read_string()] = peek(0);
      auto property = pop();
      pop();
      push(property);
      break;
    }
    case OpCode::METHOD: {
      auto name = read_string();
      auto method = peek(0);
      auto klass = obj_helpers::AsClass(peek(1));
      klass->methods[name] = method;
      pop();
      break;
    }
    case OpCode::INVOKE: {
      std::string name = read_string();
      uint8_t arg_count = read_byte();
      if (!invoke(name, arg_count)) {
        return InterpretResult::InterpretRuntimeError;
      }
      break;
    }
    case OpCode::INHERIT: {
      auto inherit_from = peek(1);
      if (!obj_helpers::IsClass(inherit_from)) {
        runtimeError("Superclass must be a class.");
      }
      auto super_class = obj_helpers::AsClass(inherit_from);

      auto sub_class = obj_helpers::AsClass(peek(0));
      sub_class->methods.insert(super_class->methods.begin(),
                                super_class->methods.end());
      pop();
      break;
    }
    default: {
      runtimeError("Unknown opcode: " + std::to_string(instruction));
      return InterpretResult::InterpretRuntimeError;
    }
    }
  }
#undef BINARY_OP
}

std::shared_ptr<ObjUpvalue> VM::captureUpvalue(uint8_t index) {
  auto prev_it = openUpvalues_.before_begin();
  auto it = openUpvalues_.begin();

  while (it != openUpvalues_.end() && (*it)->stack_idx > index) {
    ++prev_it;
    ++it;
  }

  if (it != openUpvalues_.end() && (*it)->stack_idx == index) {
    return *it;
  }

  auto upvalue = std::make_shared<ObjUpvalue>(index);
  return upvalue;
}

void VM::closeUpvalues(uint8_t index) {
  while (!openUpvalues_.empty() && openUpvalues_.front()->stack_idx >= index) {
    auto upvalue = openUpvalues_.front();
    upvalue->closed = stack_[upvalue->stack_idx];
    upvalue->stack_idx = -1;
    openUpvalues_.pop_front();
  }
}

bool VM::callValue(Value callee, uint8_t arg_count) {
  if (!Value::IsObject(callee)) {
    runtimeError("Callee need to be an object");
    return false;
  }

  switch (Value::AsObject(callee)->type) {
  case Obj::Type::CLOSURE:
    return call(obj_helpers::AsClosure(callee), arg_count);
  case Obj::Type::NATIVE: {
    auto native = obj_helpers::AsNative(callee);
    auto result =
        native(arg_count, stack_.data() + stack_.size() - arg_count - 1);
    for (int i = 0; i < arg_count; i++) {
      pop();
    }
    push(result);
    return true;
  }
  case Obj::Type::CLASS: {
    auto klass = obj_helpers::AsClass(callee);
    stack_[stack_.size() - arg_count - 1] =
        Value::Object(std::make_shared<ObjInstance>(klass));
    if (klass->methods.contains(initName)) {
      auto method = obj_helpers::AsClosure(klass->methods[initName]);
      return call(method, arg_count);
    } else if (arg_count != 0) {
      runtimeError("Expected 0 arguments but got " + std::to_string(arg_count) +
                   ".");
      return false;
    }

    return true;
  }
  case Obj::Type::BOUND_METHOD: {
    auto bound = obj_helpers::AsBoundMethod(callee);
    stack_[stack_.size() - arg_count - 1] = bound->receiver;
    return call(bound->method, arg_count);
  }
  default:
    runtimeError("Callee need to be a callable object.");
    return false;
  }
}

bool VM::invoke(const std::string &name, uint8_t arg_count) {
  auto receiver = peek(arg_count);

  if (!obj_helpers::IsInstance(receiver)) {
    runtimeError("Only instances have methods.");
    return false;
  }

  auto instance = obj_helpers::AsInstance(receiver);

  if (instance->fields.contains(name)) {
    auto value = instance->fields[name];
    stack_[stack_.size() - arg_count - 1] = value;
    return callValue(value, arg_count);
  }

  return invokeFromClass(instance->klass, name, arg_count);
}

bool VM::invokeFromClass(ObjClass *klass, const std::string &name,
                         uint8_t arg_count) {
  if (!klass->methods.contains(name)) {
    runtimeError("Undefined property '" + name + "'.");
    return false;
  }

  auto method = obj_helpers::AsClosure(klass->methods[name]);
  return call(method, arg_count);
}

bool VM::call(ObjClosure *closure, uint8_t arg_count) {
  if (arg_count != closure->function->arity) {
    runtimeError("Expected " + std::to_string(closure->function->arity) +
                 " arguments but got " + std::to_string(arg_count) + ".");
    return false;
  }

  if (frames_.size() + 1 > FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  frames_.emplace_back(CallFrame{closure, 0, stack_.size() - arg_count - 1});
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

void VM::resetStack() {
  stack_.clear();
  openUpvalues_.clear();
}

void VM::runtimeError(const std::string &message) {
  std::cerr << message << std::endl;

  for (const auto &frame : std::views::reverse(frames_)) {
    auto function = frame.closure->function;
    auto code_idx = frame.code_idx;
    auto chunk = function->chunk;
    auto line = chunk->lines[code_idx];
    auto name = function->name->str.empty() ? function->name->str : "script";
    std::cerr << "[line " << line << "] in " << name << std::endl;
  }

  resetStack();
}

bool VM::isFalsey(const Value &value) {
  return Value::IsNil(value) || (Value::IsBool(value) && !Value::AsBool(value));
}

void VM::defineNative(const std::string &name, NativeFunction function) {
  globals_[name] = Value::Object(std::make_shared<ObjNative>(function));
}

bool VM::bindMethod(ObjClass *klass, const std::string &name) {
  if (!klass->methods.contains(name)) {
    runtimeError("Undefined property '" + name + "'.");
    return false;
  }
  auto method = klass->methods[name];

  auto bound_method =
      std::make_shared<ObjBoundMethod>(peek(0), obj_helpers::AsClosure(method));

  pop();
  push(Value::Object(bound_method));
  return true;
}
