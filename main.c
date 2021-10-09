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

// static inline void set_palette_ind(int charblock, int tile, int x, int y, int ind) {
//   // clear ind
//   int u32_ind = x + y * 8;
//   tile8_mem[charblock][tile].data[u32_ind % 4] &= ~(0xffff << ((u32_ind % 4) * 8));
// }

static inline TILE8 u8s_to_tile(u8 pixels[8][8]) {
  TILE8 res;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      int u32_ind = (i + (j << 3)) >> 2;
      int u32_shift = ((i + (j << 3)) & 3) << 3;
      u8 byte = pixels[i][j];
      res.data[u32_ind] |= byte << u32_shift;
    }
  }
  return res;
}

u8 white_tile[8][8] = {
  { 0, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
};

int AgbMain(void) {
  // screen entry 0,0 to tile 1
  // se_mat[SCREENBLOCK_NUM][0][0] = 1;

  // CHARBLOCK tile x y = palette index 1
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      // tile8_mem[0][1].data[i][i] = 1;
    }
  }
  tile8_mem[CHARBLOCK_NUM][0] = u8s_to_tile(white_tile);
  // set_palette_ind(CHARBLOCK_NUM, 0, 0, 0, 1);
  // tile8_mem[CHARBLOCK_NUM][0].data[0] = 0;

  // tile8_mem[CHARBLOCK_NUM][0].data[0][1] = 1;
  // tile8_mem[0][0].data[0][2] = 1;
  // tile8_mem[0][0].data[0][3] = 1;


  pal_bg_mem[0] = RGB15(0, 0, 0);
  pal_bg_mem[1] = RGB15(24, 24, 24);

  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
  REG_BG0CNT |= BG_BASENUM(1) | BG_8BITCOL;

  for (int i = 0; ; i = (i + 1) % 8) {
    // tile8_mem[CHARBLOCK_NUM][0].data[i][i] = 0;
    // tile8_mem[CHARBLOCK_NUM][0].data[(i+1)%8][(i+1)%8] = 1;
    for (int j = 0; j < 100000; j++) {}
  }


  while (1) {}
  return 0;
}
