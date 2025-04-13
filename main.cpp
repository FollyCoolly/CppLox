#include "chunk.h"
#include "common.h"
#include "debug.h"

int main(int argc, char **argv) {
  Chunk chunk;
  int constant = chunk.AddConstant(1.2);
  chunk.Write(OpCode::Constant, 123);
  chunk.Write(constant, 123);
  chunk.Write(OpCode::Return, 124);
  disassembleChunk(chunk, "test chunk");
  return 0;
}
