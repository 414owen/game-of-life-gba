#ifndef boards_h_INCLUDED
#define boards_h_INCLUDED

#include <stdbool.h>

#define WIDTH 30
#define HEIGHT 20

typedef bool board[HEIGHT][WIDTH];

extern const board glider;
extern const board loafer;

#endif // boards_h_INCLUDED
