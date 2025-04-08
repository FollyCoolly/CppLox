#pragma once

#include "chunk.h"
#include <string_view>

void disassembleChunk(const Chunk &chunk, std::string_view name);
int disassembleInstruction(const Chunk &chunk, int offset);