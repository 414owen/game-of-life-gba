#pragma once

#include "defs.h"

typedef struct board {
  const char *name;
  const unsigned int width;
  const unsigned int height;
  const u8 *data;
};

extern const boards[];
extern const int num_boards;
