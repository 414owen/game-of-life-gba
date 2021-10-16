#ifndef rle_h_INCLUDED
#define rle_h_INCLUDED

#include <stdint.h>

typedef enum {
  NAME,
  AUTHOR,
  XY_RULE,
  INVALID,
} hash_token_type;

typedef struct {
  hash_token_type type;
  char *next;
  union {
    char *name;
    char *author;
    struct {
      unsigned long x;
      unsigned long y;
      uint8_t s;
      uint8_t b;
    };
  };
} header_token;

header_token scan_header(const char *YYCURSOR);

#endif // rle_h_INCLUDED
