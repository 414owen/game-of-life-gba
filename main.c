#include <stdbool.h>
#include <string.h>

#include "boards_compressed.h"
#include "input.h"
#include "defs.h"

#define WIDTH 30
#define HEIGHT 20

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

#define ON_TILE 0
#define OFF_TILE 31

#define MOD4(n) (n & 3)
#define MUL8(n) (n << 3)
#define DIV4(n) (n >> 2)

typedef int board[HEIGHT][WIDTH];

static board boards[2];

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

static void vid_vsync(void) {
  while(REG_VCOUNT >= 160);   // wait till VDraw
  while(REG_VCOUNT < 160);    // wait till VBlank
}

int board_ind = 0;
int other_board_ind = 1;

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
      neighbors += get_cell((x + i + WIDTH) % WIDTH, (y + j + HEIGHT) % HEIGHT) == ON_TILE ? 1 : 0;
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
    else set_cell(x, y, MIN(last + 1, 31));
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
  for (int j = 0; j < HEIGHT; j++) {
    for (int i = 0; i < WIDTH; i++) {
      update_cell(i, j);
    }
  }
}

static void setBoard(const starter *starter) {
  board_ind = 0;
  other_board_ind = 1;

  // could be flattened into one loop
  for (int n = 0; n < 2; n++) {
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        boards[n][y][x] = OFF_TILE;
      }
    }
  }
  int start_x = (WIDTH - starter->width) / 2;
  int start_y = (HEIGHT - starter->height) / 2;
  for (int y = 0; y < starter->height; y++) {
    for (int x = 0; x < starter->width; x++) {
      int ind = x + y * starter->width;
      set_cell(start_x + x, start_y + y, ((starter->data[ind / 8] & (1 << (ind % 8))) > 0) ? 0 : 31);
    }
  }
  swap_boards();
}

void interruptHandler();

// This is the function that will be called by the CPU when an interrupt is triggered
void interruptHandler() {
	REG_IF = INT_VBLANK;
	REG_IFBIOS |= INT_VBLANK;
}

// This is the declaration for the function we will call to trigger the VBLANK interrupt wait
void vblank_intr_wait() {
  // 0x5 is VBlankIntrWait in the function table.
  asm volatile("swi 0x02");
}

// This is the function that we wil call to register that we want a VBLANK Interrupt
void register_vblank_isr() {
	REG_IME = 0x00;
	REG_INTERRUPT = (fnptr)interruptHandler;
	REG_DISPSTAT |= DSTAT_VBL_IRQ;
	REG_IE |= INT_VBLANK;
	REG_IME = 1;
}

extern void halt(void);

int AgbMain(void) {

  int board_ind = 0;

  setBoard(&starters[board_ind]);

  // screen entry 0,0 to tile 1
  // se_mat[SCREENBLOCK_NUM][19][29] = 31;

  // create a flat tile for 'on' cells
  for (int i = 0; i < 32; i++) {
    int chan = 16 - i / 2;
    pal_bg_mem[i] = RGB15(chan, chan, chan);
    tile8_mem[CHARBLOCK_NUM][i] = make_flat_tile(i);
  }

  pal_bg_mem[0] = RGB15(31, 10, 10);
  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
  REG_BG0CNT |= BG_BASENUM(1) | BG_8BITCOL;

  int delay = 6;
  while (1) {
    key_poll();
    display();
    update();
    swap_boards();
    if (key_hit(KEY_L)) delay++;
    if (key_hit(KEY_R)) delay = MAX(delay - 1, 1);
    if (key_hit(KEY_UP)) {
      board_ind = (board_ind + 1) % num_starters;
      setBoard(&starters[board_ind]);
    }
    if (key_hit(KEY_DOWN)) {
      board_ind = (board_ind + num_starters - 1) % num_starters;
      setBoard(&starters[board_ind]);
    }
    for (int i = 0; i < delay; i++) {
      register_vblank_isr();
      halt();
    }
  }

  return 0;
}
