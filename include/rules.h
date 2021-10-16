#ifndef rules_h_INCLUDED
#define rules_h_INCLUDED

#include "defs.h"

typedef struct {
  const char *name;
  const char *creator;
  // list of digits
  u8 stay_alive_rules;
  u8 birth_rules;
  int width;
  int height;
  union {
    const char *rle;
    const unsigned char *packed;
  };
} rule;

#endif
