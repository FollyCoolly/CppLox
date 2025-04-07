#include "chunk.h"
#include "common.h"

int main(int argc, char **argv) {
  Chunk chunk;
  chunk.Write(static_cast<uint8_t>(OpCode::Return));
  return 0;
}
