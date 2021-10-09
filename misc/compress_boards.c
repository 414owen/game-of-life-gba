#include <stdbool.h>
#include <stdio.h>

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define static_len(sym) (sizeof(sym) / sizeof(sym[0]))

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#include "../include/boards.h"

typedef struct {
  char *name;
  const board *board;
  int width;
  int height;
} board_conf;

board_conf boards[] = {
  { .name = "loafer", .board = &loafer },
  { .name = "glider", .board = &glider },
};

int main(int argc, char **argv) {

  FILE *c = fopen("src/boards_compressed.c", "w");

  fputs("", c);
  fprintf(c, "#include \"../include/boards_compressed.h\"\n\n"
        "const int num_starters = %lu;\n", static_len(boards));

  for (int b = 0; b < static_len(boards); b++) {
    printf("Processing board %s\n", boards[b].name);

    int min_x = WIDTH;
    int max_x = 0;
    int min_y = HEIGHT;
    int max_y = 0;
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        if ((*(boards[b].board))[y][x]) {
          min_x = MIN(min_x, x);
          min_y = MIN(min_y, y);
          max_x = MAX(max_x, x);
          max_y = MAX(max_y, y);
        }
      }
    }

    int width = MAX(0, 1 + max_x - min_x);
    int height = MAX(0, 1 + max_y - min_y);
    boards[b].width = width;
    boards[b].height = height;

    fprintf(c, "const u8 %s_data[%d] = {",
      boards[b].name,
      width * height / 8 + ((width * height % 8 == 0) ? 0 : 1));

    unsigned char byte = 0;
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int i = y * width + x;
        printf("\r%d/%d", i + 1, width * height);
        fflush(stdout);
        byte |= (*(boards[b].board))[min_y + y][min_x + x] << (i % 8);
        if (i % 8 == 7) {
          fprintf(c, "0x%hhx,", byte);
          byte = 0;
        }
      }
    }

    bool byte_divides_board = (width * height) % 8 == 0;
    if (!byte_divides_board) fprintf(c, "0x%hhx,", byte);
    fputs("};\n", c);


    puts("");
  }

  fputs("const starter starters[] = {\n", c);

  for (int b = 0; b < static_len(boards); b++) {
    fprintf(c, "  {.name = \"%s\", .width = %d, .height = %d, .data = %s_data},\n",
               boards[b].name,
               boards[b].width,
               boards[b].height,
               boards[b].name);
  }
  fputs("};", c);

  fclose(c);
}
