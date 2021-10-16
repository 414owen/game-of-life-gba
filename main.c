#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "rle_boards.h"
#include "packed_boards.h"
#include "rules.h"
#include "input.h"
#include "defs.h"

#define WIDTH 30
#define HEIGHT 20

// max 255, as we have 256 pallette entries
#define TAIL_FRAMES 31

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

#define MOD4(n) (n & 3)
#define MUL8(n) (n << 3)
#define DIV4(n) (n >> 2)

typedef int board[HEIGHT][WIDTH];
static board boards[2];

typedef struct {
  bool is_rle;
  rule *r;
} full_rule;

// Create a 15bit BGR color.
// TODO test can we use | instead of +?
static COLOR RGB15(int red, int green, int blue) {
  return red + (green<<5) + (blue<<10);
}

static TILE8 make_flat_tile(u8 pallette_ind) {
  TILE8 res;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      res.bytes[i][j] = pallette_ind;
    }
  }
  return res;
}

static int board_ind = 0;
static int other_board_ind = 1;

// write to board, read from other
static void set_cell(int x, int y, int frames_since_alive) {
  boards[other_board_ind][y][x] = frames_since_alive;
}

static int get_cell(int x, int y) {
  return boards[board_ind][y][x];
}

static void update_cell(int x, int y) {
  int neighbors = 0;
  for (int j = -1; j <= 1; j++) {
    for (int i = -1; i <= 1; i++) {
      neighbors += get_cell((x + i + WIDTH) % WIDTH, (y + j + HEIGHT) % HEIGHT) == 0 ? 1 : 0;
    }
  }

  // always have to set every cell, even if unchanged
  int last = get_cell(x, y);
  if (last == 0) {
    neighbors--;
    if (neighbors == 2 || neighbors == 3) {
      set_cell(x, y, 0);
    } else {
      set_cell(x, y, 1);
    }
  } else {
    if (neighbors == 3) set_cell(x, y, 0);
    else set_cell(x, y, last);
  }
}

static void update_cell_tail(int x, int y) {
  int last = get_cell(x, y);
  if (last == 0) {
    set_cell(x, y, 0);
  } else {
    set_cell(x, y, MIN(last + 1, TAIL_FRAMES));
  }
}

static void swap_boards(void) {
  int tmp = board_ind;
  board_ind = other_board_ind;
  other_board_ind = tmp;
}

static void display(void) {
  for (int j = 0; j < HEIGHT; j++) {
    for (int i = 0; i < WIDTH; i++) {
      se_mat[SCREENBLOCK_NUM][j][i] = get_cell(i, j);
    }
  }
}

static void update(void) {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      update_cell(x, y);
    }
  }
}

static void update_tail_frames(void) {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      update_cell_tail(x, y);
    }
  }
}

static void set_board_packed(int width, int height, unsigned char *bits) {

  boards[0][0][0] = 0;
  boards[0][0][1] = 0;
  boards[0][0][2] = 0;

  return;

  int start_x = (WIDTH - width) / 2;
  int start_y = (HEIGHT - height) / 2;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int ind = x + y * width;
      set_cell(start_x + x, start_y + y, ((bits[ind / 8] & (1 << (ind % 8))) > 0) ? 0 : TAIL_FRAMES);
    }
  }
}

static void set_board_rle(int width, int height, char *cursor) {

  int start_x = (WIDTH - width) / 2;
  int y = (HEIGHT - height) / 2;
  int x = start_x;
  int n = 0;
  char c = *cursor;
  while (c != '\0') {
    if (isdigit(c)) {
      n *= 10;
      n += c - '0';
    } else if (c == 'o') {
      n = MAX(1, n);
      while (n) {
        set_cell(x, y, true);
        x++;
        n--;
      }
    } else if (c == 'b') {
      x += MAX(1, n);
      n = 0;
    } else if (c == '$') {
      x = start_x;
      y += MAX(1, n);
      n = 0;
    }
    c = *(++cursor);
  }

}

static void setBoard(full_rule starter) {
  board_ind = 0;
  other_board_ind = 1;

  // could be flattened into one loop
  for (int n = 0; n < 2; n++) {
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        boards[n][y][x] = TAIL_FRAMES;
      }
    }
  }

  if (starter.is_rle) {
    set_board_rle(starter.r->width, starter.r->height, starter.r->rle);
  } else {
    set_board_packed(starter.r->width, starter.r->height, starter.r->packed);
  }
}

// This is the function that will be called by the CPU when an interrupt is triggered
static void interruptHandler() {
	REG_IF = INT_VBLANK;
	REG_IFBIOS |= INT_VBLANK;
}

// This is the function that we wil call to register that we want a VBLANK Interrupt
static void register_vblank_isr() {
	REG_IME = 0x00;
	REG_INTERRUPT = (fnptr)interruptHandler;
	REG_DISPSTAT |= DSTAT_VBL_IRQ;
	REG_IE |= INT_VBLANK;
	REG_IME = 1;
}

extern void halt(void);

static full_rule get_board(int i) {
  full_rule res;
  if (i < rle_rule_amt) {
    res.is_rle = true;
    res.r = &rle_rules[i];
  } else {
    res.is_rle = false;
    res.r = &packed_rules[i - rle_rule_amt];
  }
  return res;
}

int AgbMain(void) {

  int board_ind = 0;
  const int num_starters = rle_rule_amt + packed_rule_amt;

  setBoard(get_board(board_ind));

  // screen entry 0,0 to tile 1
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      se_mat[SCREENBLOCK_NUM][y][x] = 31;
    }
  }

  // create a flat tile for 'on' cells
  for (int i = 0; i <= TAIL_FRAMES; i++) {
    int r = abs((i + 100 / 2 % TAIL_FRAMES) - TAIL_FRAMES / 2) * 64 / TAIL_FRAMES;
    int g = abs(((i / 2 + TAIL_FRAMES / 3) % TAIL_FRAMES) - TAIL_FRAMES / 2) * 64 / TAIL_FRAMES;
    int b = abs(((i / TAIL_FRAMES / 3) % TAIL_FRAMES) - TAIL_FRAMES / 2) * 64 / TAIL_FRAMES;
    pal_bg_mem[TAIL_FRAMES - i] = RGB15(r / 3,g / 3,b / 3);
    tile8_mem[CHARBLOCK_NUM][i] = make_flat_tile(i);
  }

  pal_bg_mem[0] = RGB15(31, 10, 10);
  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
  REG_BG0CNT |= BG_BASENUM(1) | BG_8BITCOL;

  // while (true) {}
  // set_cell(0, 0, 0);
  // swap_boards();
  // display();
  // return 1;

  // could be flattened into one loop
  // for (int n = 0; n < 2; n++) {
  //   for (int y = 0; y < HEIGHT; y++) {
  //     for (int x = 0; x < WIDTH; x++) {
  //       boards[n][y][x] = 0;
  //     }
  //   }
  // }

  // unsigned char *test = { 0x00 };

  int delay = 6;
  while (1) {
    update();
    swap_boards();
    for (int i = 0; i < delay; i++) {
      key_poll();
      if (key_hit(KEY_L)) delay++;
      if (key_hit(KEY_R)) delay = MAX(delay - 1, 1);
      if (key_hit(KEY_UP)) {
        board_ind = (board_ind + 1) % num_starters;
        setBoard(get_board(board_ind));
      }
      if (key_hit(KEY_DOWN)) {
        board_ind = (board_ind + num_starters - 1) % num_starters;
        setBoard(get_board(board_ind));
      }
      display();
      update_tail_frames();
      swap_boards();
      register_vblank_isr();
      halt();
    }
  }

  return 0;
}
