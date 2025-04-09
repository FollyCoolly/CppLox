#include "chunk.h"
#include "common.h"
#include "debug.h"

int main(int argc, char **argv) {
  Chunk chunk;
  int constant = chunk.AddConstant(1.2);
  chunk.Write(static_cast<uint8_t>(OpCode::Constant));
  chunk.Write(constant);
  disassembleChunk(chunk, "test chunk");
  return 0;
}
