#ifndef include/rle_h_INCLUDED
#define include/rle_h_INCLUDED

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

#endif // include/rle_h_INCLUDED
