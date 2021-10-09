#include "defs.h"

/*
 *  char blocks        screen blocks
 * |-----------|       |-----------|
 * |           |       |           |
 * |     1     |       |    8-15   |
 * |           |       |           |
 * |-----------|       |-----------|
 * |           |       |           |
 * |     0     |       |    0-7    |
 * |           |       |           |
 * |-----------|       |-----------|
 *
 * Screen data = tile references
 * Char data = 8x8 palette references
 *
 * We'll use char block 1, screen block 0
 */

#define CHARBLOCK_NUM 1
#define SCREENBLOCK_NUM 0

// Create a 15bit BGR color.
// TODO test can we use | instead of +?
static inline COLOR RGB15(int red, int green, int blue) {
  return red + (green<<5) + (blue<<10);
}

int AgbMain(void) {
  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
  while (1) {}
  return 0;
}
