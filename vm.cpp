#include "vm.h"
#include <iostream>

VM &VM::getInstance() {
  static VM instance;
  return instance;
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

  while (true) {
    uint8_t instruction = readByte();
    switch (from_uint8(instruction)) {
    case OpCode::Return:
      return InterpretResult::InterpretOk;
    case OpCode::Constant:
      Value constant = readConstant();
      std::cout << constant << std::endl;
      break;
    }
  }
}
