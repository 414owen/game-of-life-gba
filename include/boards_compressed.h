#pragma once

#include "defs.h"

typedef struct {
  const char *name;
  const unsigned int width;
  const unsigned int height;
  const u8 *data;
} starter;

extern const starter starters[];
extern const int num_starters;
