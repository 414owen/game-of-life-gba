#ifndef pattern_h_INCLUDED
#define pattern_h_INCLUDED

#include "defs.h"

struct rule {
  // bitset of whether [0-8] neighbours means we survive
  u8 stay_alive;
  u8 birth;
};

typedef struct {
  struct rule rule;
  int width;
  int height;
  union {
    const char *rle;
    const unsigned char *packed;
  };
  const char *creator;
  const char *name;
} pattern;

#endif
