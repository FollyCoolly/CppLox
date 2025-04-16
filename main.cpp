#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, char **argv) {
  auto chunk = std::make_shared<Chunk>();
  int constant = chunk->AddConstant(1.2);
  chunk->Write(OpCode::Constant, 123);
  chunk->Write(constant, 123);
  chunk->Write(OpCode::Return, 124);
  // disassembleChunk(*chunk, "test chunk");
  VM::getInstance().interpret(chunk);
  return 0;
}
