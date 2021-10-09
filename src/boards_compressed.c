#include "../include/boards_compressed.h"

const int num_starters = 2;
const u8 loafer_data[11] = {0xa6,0x93,0x29,0x20,0x0,0x10,0x38,0x8,0x20,0x80,0x1,};
const u8 glider_data[2] = {0xac,0x1,};
const starter starters[] = {
  {.name = "loafer", .width = 9, .height = 9, .data = loafer_data},
  {.name = "glider", .width = 3, .height = 3, .data = glider_data},
};