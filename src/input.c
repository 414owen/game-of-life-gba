#include <stdbool.h>

#include "defs.h"

u16 key_curr;
u16 key_prev;

u32 key_is_down(u32 key) {
  return key_curr & key;
}

void key_poll(void) {
  key_prev = key_curr;
  key_curr = ~REG_KEYINPUT & KEY_MASK;
}

bool key_hit(u32 key) {
  return ((key_curr & ~key_prev) & key) > 0;
}
