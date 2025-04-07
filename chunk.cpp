#include "chunk.h"

void Chunk::Write(uint8_t byte) {
  if (capacity_ < count_ + 1) {
    int old_capacity = capacity_;
    capacity_ = GROW_CAPACITY(old_capacity);
    code_ = GROW_ARRAY(uint8_t, code_, old_capacity, capacity_);
  }

  code_[count_] = byte;
  count_++;
}
