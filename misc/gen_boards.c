#include <ctype.h>
#include <dirent.h>
#include <inttypes.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "rules.h"
#include "rle.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

char *read_file(char *fname, size_t *len) {
  FILE *f = fopen(fname, "rb");
  if (f == NULL) {
    perror("Error opening file");
    exit(1);
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  if (fsize == -1) {
    perror("Error getting file size");
    exit(1);
  }
  *len = (size_t) fsize;
  char *string = malloc(fsize + 1);
  rewind(f);
  if (fread(string, 1, fsize, f) != fsize) {
    perror(strerror(errno));
    exit(1);
  }
  fclose(f);
  string[fsize] = '\0';
  return string;
}

// void str_or_null(char *s) {
//   if (s != NULL) return s;
//   static const char *nullstr = "NULL";
//   static const nulllen = strlen(nullstr);
//   char *res = malloc(nulllen + 1);
//   strcpy(res, nullstr);
//   res[nulllen] = '\0';
//   return res;
// }

FILE *rle_out;
FILE *packed_out;

int successes = 0;
int invalids = 0;
int bad_dims = 0;

int rle_amt = 0;
int packed_amt = 0;

int rle_bytes_used = 0;
int packed_bytes_used = 0;
int both_bytes = 0;

void fprint_common(FILE *out, rule r) {
  fputs("  {\n    .name = ", out);
  if (r.name == NULL) {
    fputs("NULL", out);
  } else {
    fprintf(out, "\"%s\"", r.name);
  }
  fputs(",\n    .creator = ", out);

  if (r.creator == NULL) {
    fputs("NULL", out);
  } else {
    fprintf(out, "\"%s\"", r.creator);
  }

  fprintf(out, ",\n"
    "    .birth_rules = %" PRIu8 ",\n"
    "    .stay_alive_rules = %" PRIu8 ",\n"
    "    .width = %d,\n"
    "    .height = %d,\n"
    "    .data = \"",
    r.birth_rules,
    r.stay_alive_rules,
    r.width,
    r.height
  );
}

void fprint_rle(rule r) {
  fprint_common(rle_out, r);
  fprintf(rle_out, "%s\"\n  },\n", r.data);
}

int bitset_bytes(int bools) {
  int res = bools/8;
  if (bools % 8 > 0) res++;
  return res;
}

uint8_t *board_bitset(int width, int height) {
  void *res = calloc(bitset_bytes(width*height), 1);
  return (uint8_t*) res;
}

void fprint_packed(rule r) {
  fprint_common(packed_out, r);

  bool board[20][30];
  memset(board, 0, 20 * 30 * sizeof(bool));
  int x = 0;
  int y = 0;
  int n = 0;
  for (int i = 0; i < strlen(r.data); i++) {
    char c = r.data[i];
    if (isdigit(c)) {
      n *= 10;
      n += c - '0';
    } else {
      if (c == 'o') {
        n = MAX(n, 1);
        while (n) {
          board[y][x] = true;
          x += 0;
          n--;
        }
      } else if (c == 'b') {
        x += MAX(1, n);
      } else if (c == '$') {
        x = 0;
        y += MAX(1, n);
      }
      n = 0;
    }
  }

  int pos = 0;
  uint8_t *res = board_bitset(r.width, r.height);
  for (int y = 0; y < r.height; y++) {
    for (int x = 0; x < r.width; x++) {
      res[pos / 8] |= ((uint8_t) board[y][x]) << (pos % 8);
      pos++;
    }
  }

  for (int i = 0; i < bitset_bytes(r.width * r.height); i++) {
    fprintf(packed_out, "\\x%x", res[i]);
  }

  fprintf(packed_out, "\"\n  },\n");
  free(res);
}

void get_dims(char *cursor, int *width, int *height) {
  int x = 0;
  int y = 0;
  int n = 0;
  int max_width = 0;
  char c = *cursor;
  while (c != '\0') {
    if (isdigit(c)) {
      n *= 10;
      n += c - '0';
    } else if (c == 'o') {
      x += MAX(1, n);
      max_width = MAX(max_width, x) + 1;
      n = 0;
    } else if (c == 'b') {
      x += MAX(1, n);
      n = 0;
    } else if (c == '$') {
      x = 0;
      y += MAX(1, n);
      n = 0;
    }
    c = *(++cursor);
  }
  *width = max_width;
  *height = y + 1;
}

void safe_free(void *a) {
  if (a == NULL) return;
  free(a);
}

void free_rule(rule r) {
  safe_free(r.creator);
  safe_free(r.name);
}

void run_file(char *fname) {
  rule r = {
    .creator = NULL,
    .name = NULL
  };
  // printf("\rparsing file %s\n", fname);
  size_t file_len = 0;
  char *file = read_file(fname, &file_len);
  char *cursor = file;

  int header_lines_parsed = 0;
  header_token t;
  do {

    t = scan_header(cursor);

    if (t.type == INVALID) {
      invalids++;
      free_rule(r);
      free(file);
      return;
    }

    header_lines_parsed++;
    switch (t.type) {
      case NAME:
        if (r.name != NULL) free(r.name);
        r.name = t.name;
        break;
      case AUTHOR:
        if (r.creator != NULL) free(r.creator);
        r.creator = t.author;
        break;
      case XY_RULE:
        // r.x = t.x;
        // r.y = t.y;
        r.stay_alive_rules = t.s;
        r.birth_rules = t.b;
        break;
      case INVALID:
        // impossible
        break;
    }

    cursor = t.next;
  } while (t.type != XY_RULE);

  r.data = cursor;
  get_dims(cursor, &r.width, &r.height);

  if (r.width == 0 || r.width > 30 || r.height == 0 || r.height > 20) {
    bad_dims++;
  } else {
    successes++;
    int PTR_SIZE = 4;
    int our_rle_bytes = PTR_SIZE + strlen(r.data) + 1;
    rle_bytes_used += our_rle_bytes;
    int area = r.width * r.height;
    int our_packed_bytes_used = area / 8 + (area % 8 == 0 ? 0 : 1);
    packed_bytes_used += PTR_SIZE + our_packed_bytes_used;
    both_bytes += MIN(our_packed_bytes_used, our_rle_bytes);

    if (our_rle_bytes < our_packed_bytes_used) {
      rle_amt++;
      fprint_rle(r);
    } else {
      packed_amt++;
      fprint_packed(r);
    }
  }
  free(file);
  free_rule(r);
}

int main(int argc, char **argv) {
  DIR *d;
  struct dirent *dir;
  char *dir_prefix = "all/";
  size_t dir_len = strlen(dir_prefix);
  d = opendir(dir_prefix);
  if (!d) {
    perror("Coldn't open directory 'all'");
    exit(1);
  }

  // shitty debug mode
  // rle_out = stdout;
  // packed_out = stdout;

  rle_out = fopen("gen/boards_rle.c", "w");
  packed_out = fopen("gen/boards_packed.c", "w");

  char *intro = "#include \"rules.h\"\n#include <stddef.h>\n";
  fputs(intro, rle_out);
  fputs(intro, packed_out);
  fputs("rule rle_rules[] = {\n", rle_out);
  fputs("rule packed_rules[] = {\n", packed_out);

  while ((dir = readdir(d)) != NULL) {
    if (dir->d_type != DT_REG) continue;
    size_t len = strlen(dir->d_name);
    char *fname = malloc(len + dir_len + 1);
    strcpy(fname, dir_prefix);
    strcpy(fname + dir_len, dir->d_name);
    fname[len + dir_len] = '\0';
    puts(fname);
    run_file(fname);
    free(fname);
  }

  fprintf(rle_out, "};\n\nint rle_rule_amt = %d;\n", rle_amt);
  fprintf(packed_out, "};\n\nint packed_rule_amt = %d;\n", packed_amt);

  printf("successes: %d\n"
    "invalids: %d\n"
    "bad_dims: %d\n"
    "rle_bytes_used: %d\n"
    "packed_bytes_used: %d\n"
    "both_bytes: %d\n\n"
    "packed as percent of rle: %d\n",
    successes, invalids, bad_dims, rle_bytes_used, packed_bytes_used, both_bytes,
    100 * packed_bytes_used / MAX(rle_bytes_used, 1)
  );

  fflush(rle_out);
  fflush(packed_out);

  fclose(rle_out);
  fclose(packed_out);

  closedir(d);
}
