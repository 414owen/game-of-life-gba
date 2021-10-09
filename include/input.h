#ifndef input_h_INCLUDED
#define input_h_INCLUDED

#include <stdbool.h>

#include "defs.h"

#define KEY_DOWN_NOW(key) (~(REG_KEYINPUT) & key)
#define KEY_UP_NOW(key)   ( (REG_KEYINPUT) & key)

void key_poll(void);

u32 key_is_down(u32 key);

bool key_hit(u32 key);

#endif // input_h_INCLUDED
