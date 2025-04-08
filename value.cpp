#include "value.h"

void ValueArray::Write(Value value) {
  if (capacity < count + 1) {
    int oldCapacity = capacity;
    capacity = GROW_CAPACITY(oldCapacity);
    values = GROW_ARRAY(Value, values, oldCapacity, capacity);
  }

  values[count] = value;
  count++;
}
