#include <stdbool.h>
#include "boards.h"
#include "defs.h"
#include <string.h>

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

#define ON_TILE 1
#define OFF_TILE 0

static board boards[2];

// Create a 15bit BGR color.
// TODO test can we use | instead of +?
static COLOR RGB15(int red, int green, int blue) {
  return red + (green<<5) + (blue<<10);
}

static TILE8 u8s_to_tile(u8 pixels[8][8]) {
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

static TILE8 make_flat_tile(u8 pallette_ind) {
  u8 res[8][8];
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      res[i][j] = pallette_ind;
    }
  }
  return u8s_to_tile(res);
}

static void vid_vsync(void) {
  while(REG_VCOUNT >= 160);   // wait till VDraw
  while(REG_VCOUNT < 160);    // wait till VBlank
}

int board_ind = 0;
int other_board_ind = 1;

// write to board, read from other
static void set_cell(int x, int y, bool on) {
  boards[other_board_ind][y][x] = on;
}

static bool get_cell(int x, int y) {
  return boards[board_ind][y][x];
}

static void update_cell(int x, int y) {
  int neighbors = 0;
  for (int j = -1; j <= 1; j++) {
    for (int i = -1; i <= 1; i++) {
      neighbors += get_cell((x + i + WIDTH) % WIDTH, (y + j + HEIGHT) % HEIGHT) ? 1 : 0;
    }
  }

  if (get_cell(x, y)) {
    neighbors--;
    set_cell(x, y, neighbors == 2 || neighbors == 3);
  } else {
    set_cell(x, y, neighbors == 3);
  }
  // set_cell(x, y, get_cell(x, y));
  // if(!get_cell(x, y)) set_cell(x, y, neighbors == 3);
  // if (neighbors == 1) set_cell(x, y, true);
}

static void swap_boards(void) {
  int tmp = board_ind;
  board_ind = other_board_ind;
  other_board_ind = tmp;
}

static void display(void) {
  for (int j = 0; j < HEIGHT; j++) {
    for (int i = 0; i < WIDTH; i++) {
      se_mat[SCREENBLOCK_NUM][j][i] = get_cell(i, j) ? ON_TILE : OFF_TILE;
    }
  }
}

static void update(void) {
  for (int j = 0; j < HEIGHT; j++) {
    for (int i = 0; i < WIDTH; i++) {
      update_cell(i, j);
    }
  }
  swap_boards();
  display();
}

static void setBoard(const bool state[HEIGHT][WIDTH]) {
  memcpy(boards[0], state, sizeof(bool) * WIDTH * HEIGHT);
}

int AgbMain(void) {
  setBoard(glider);

  // screen entry 0,0 to tile 1
  se_mat[SCREENBLOCK_NUM][19][29] = 1;

  // create a flat tile for 'on' cells
  tile8_mem[CHARBLOCK_NUM][1] = make_flat_tile(1);

  pal_bg_mem[1] = RGB15(31, 31, 31);
  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
  REG_BG0CNT |= BG_BASENUM(1) | BG_8BITCOL;

  display();
  while (true) {
    update();
    for (int i = 0; i < 6; i++) vid_vsync();
  }

  return 0;
}
