#ifndef rules_h_INCLUDED
#define rules_h_INCLUDED

#include "defs.h"

typedef struct {
  char *name;
  char *creator;
  // list of digits
  u8 stay_alive_rules;
  u8 birth_rules;
  int x;
  int y;
} rule;

#endif
