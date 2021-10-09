#include "../include/boards_compressed.h"

const int num_boards = 2;
const u8 loafer_data[0] = {0x65,0xc9,0x94,0x4,0x0,0x8,0x1c,0x10,0x4,0x1,};
const u8 glider_data[0] = {0x35,};
const board boards[] = {
  {.name = "loafer", .width = 9, .height = 9, .data = loafer_data},
  {.name = "glider", .width = 3, .height = 3, .data = glider_data},
};