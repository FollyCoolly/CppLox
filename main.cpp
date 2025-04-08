#include "chunk.h"
#include "common.h"
#include "debug.h"

int main(int argc, char **argv) {
  Chunk chunk;
  chunk.Write(static_cast<uint8_t>(OpCode::Return));
  disassembleChunk(chunk, "test chunk");
  return 0;
}
