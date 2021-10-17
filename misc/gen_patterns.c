#include <ctype.h>
#include <dirent.h>
#include <inttypes.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "pattern.h"
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
int bad_rules = 0;

int rle_amt = 0;
int packed_amt = 0;

int rle_bytes_used = 0;
int packed_bytes_used = 0;
int both_bytes = 0;

void fprint_common(FILE *out, pattern p) {
  fputs("  {\n    .name = ", out);
  if (p.name == NULL) {
    fputs("NULL", out);
  } else {
    fprintf(out, "\"%s\"", p.name);
  }
  fputs(",\n    .creator = ", out);

  if (p.creator == NULL) {
    fputs("NULL", out);
  } else {
    fprintf(out, "\"%s\"", p.creator);
  }

  fprintf(out, ",\n"
    "    .rule = {\n"
    "      .birth = %" PRIu8 ",\n"
    "      .stay_alive = %" PRIu8 ",\n"
    "    },\n"
    "    .width = %d,\n"
    "    .height = %d,\n",
    p.rule.birth,
    p.rule.stay_alive,
    p.width,
    p.height
  );
}

void fprint_rle(pattern p) {
  fprint_common(rle_out, p);
  fprintf(rle_out, "    .rle = \"%s\"\n  },\n", p.rle);
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

void fprint_packed(pattern p) {
  fprint_common(packed_out, p);
  fprintf(packed_out, "    .packed = packed_data_%d\n  },\n", packed_amt);
}

bool fits(pattern p) {
  return p.width > 0 && p.width <= 30 && p.height > 0 && p.height <= 20;
}

void fprint_packed_data(pattern p) {
  int bb = bitset_bytes(p.width * p.height);
  fprintf(packed_out, "const unsigned char packed_data_%d[%d] = {", packed_amt, bb);
  bool board[20][30];
  memset(board, 0, 20 * 30 * sizeof(bool));
  int x = 0;
  int y = 0;
  int n = 0;
  for (int i = 0; i < strlen(p.rle); i++) {
    char c = p.rle[i];
    // printf("%c\n", c);
    if (isdigit(c)) {
      n *= 10;
      n += c - '0';
    } else {
      if (c == 'o') {
        n = MAX(n, 1);
        while (n) {
          board[y][x] = true;
          x += 1;
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

  uint8_t *res = board_bitset(p.width, p.height);
  for (int y = 0; y < p.height; y++) {
    for (int x = 0; x < p.width; x++) {
      int pos = x + y * p.width;
      // printf("x: %d, y: %d, on: %d, pos: %d\n", x, y, board[y][x], pos / 8);
      // printf("before: 0x%02x\n", res[pos / 8]);
      res[pos / 8] |= ((board[y][x] ? 1 : 0) << (pos % 8));
      // printf("after: 0x%02x\n", res[pos / 8]);
    }
  }

  for (int i = 0; i < bb; i++) {
    if (i % 8 == 0) fputs("\n ", packed_out);
    fprintf(packed_out, " 0x%02x,", res[i]);
  }

  fprintf(packed_out, "\n};\n\n");
  free(res);
}

void get_dims(const char *cursor, int *width, int *height) {
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
      max_width = MAX(max_width, x + 1);
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

void free_pattern(pattern p) {
  safe_free((void*) p.creator);
  safe_free((void*) p.name);
  free((void*) p.rle);
}

pattern parse_headers(char *fname, bool *success) {
  pattern p = {
    .creator = NULL,
    .name = NULL
  };

  size_t file_len = 0;
  char *file = read_file(fname, &file_len);
  char *cursor = file;

  int header_lines_parsed = 0;
  header_token t;
  do {

    t = scan_header(cursor);

    if (t.type == INVALID) {
      invalids++;
      *success = false;
      free_pattern(p);
      return p;
    }

    header_lines_parsed++;
    switch (t.type) {
      case NAME:
        if (p.name != NULL) free((void*) p.name);
        p.name = t.name;
        break;
      case AUTHOR:
        if (p.creator != NULL) free((void*) p.creator);
        p.creator = t.author;
        break;
      case XY_RULE:
        // p.x = t.x;
        // p.y = t.y;
        p.rule.stay_alive = t.s;
        p.rule.birth = t.b;
        break;
      case INVALID:
        // impossible
        break;
    }

    cursor = t.next;
  } while (t.type != XY_RULE);

  *success = true;
  p.rle = strdup(cursor);
  free(file);
  return p;
}

bool valid_ruleset(pattern p) {
  return p.rule.stay_alive == 12 && p.rule.birth == 8;
}

void print_packed_data(char *fname) {
  bool success;
  pattern p = parse_headers(fname, &success);
  if (!success) return;

  get_dims(p.rle, &p.width, &p.height);
  if (!(fits(p) && valid_ruleset(p))) return;

  int area = p.width * p.height;
  int our_rle_bytes = strlen(p.rle) + 1;
  int our_packed_bytes = bitset_bytes(area);
  if (our_rle_bytes > our_packed_bytes) {
    fprint_packed_data(p);
    packed_amt++;
  }
}

void run_file(char *fname) {
  bool success;
  pattern p = parse_headers(fname, &success);
  if (!success) return;

  get_dims(p.rle, &p.width, &p.height);
  if (!fits(p)) {
    bad_dims++;
    free_pattern(p);
    return;
  }

  if (!valid_ruleset(p)) {
    bad_rules++;
    free_pattern(p);
    return;
  }

  successes++;
  int PTR_SIZE = 4;
  int our_rle_bytes = PTR_SIZE + strlen(p.rle) + 1;
  rle_bytes_used += our_rle_bytes;
  int area = p.width * p.height;
  int our_packed_bytes = PTR_SIZE + area / 8 + (area % 8 == 0 ? 0 : 1);
  packed_bytes_used += our_packed_bytes;
  both_bytes += MIN(our_packed_bytes, our_rle_bytes);

  if (our_rle_bytes < our_packed_bytes) {
    fprint_rle(p);
    rle_amt++;
  } else {
    fprint_packed(p);
    packed_amt++;
  }
  free_pattern(p);
}

char *dummy_pattern = "{}";

int main(int argc, char **argv) {
  DIR *d;
  struct dirent *dir;
  char *dir_prefix = "all/";
  size_t dir_len = strlen(dir_prefix);

  // shitty debug mode
  // rle_out = stdout;
  // packed_out = stdout;

  rle_out = fopen("gen/patterns_rle.c", "w");
  packed_out = fopen("gen/patterns_packed.c", "w");

  char *intro = "#include \"pattern.h\"\n#include <stddef.h>\n\n";
  fputs(intro, rle_out);
  fputs(intro, packed_out);

  {
    d = opendir(dir_prefix);
    if (!d) {
      perror("Coldn't open directory 'all'");
      exit(1);
    }

    while ((dir = readdir(d)) != NULL) {
      if (dir->d_type != DT_REG) continue;
      size_t len = strlen(dir->d_name);
      char *fname = malloc(len + dir_len + 1);
      strcpy(fname, dir_prefix);
      strcpy(fname + dir_len, dir->d_name);
      fname[len + dir_len] = '\0';
      // puts(fname);
      print_packed_data(fname);
      free(fname);
    }

    packed_amt = 0;
    closedir(d);
  }

  fputs("const pattern rle_pattern_arr[] = {\n", rle_out);
  fputs("const pattern packed_pattern_arr[] = {\n", packed_out);

  {
    d = opendir(dir_prefix);
    if (!d) {
      perror("Coldn't open directory 'all'");
      exit(1);
    }

    while ((dir = readdir(d)) != NULL) {
      if (dir->d_type != DT_REG) continue;
      size_t len = strlen(dir->d_name);
      char *fname = malloc(len + dir_len + 1);
      strcpy(fname, dir_prefix);
      strcpy(fname + dir_len, dir->d_name);
      fname[len + dir_len] = '\0';
      // puts(fname);
      run_file(fname);
      free(fname);
    }

    closedir(d);
  }

  if (rle_amt == 0) {
    fputs(dummy_pattern, rle_out);
  }

  if (packed_amt == 0) {
    fputs(dummy_pattern, packed_out);
  }

  fprintf(rle_out, "};\n\nconst unsigned int rle_pattern_amt = %d;\n\n", rle_amt);
  fprintf(packed_out, "};\n\nconst unsigned int packed_pattern_amt = %d;\n\n", packed_amt);
  fputs("const pattern *rle_patterns = &rle_pattern_arr[0];\n", rle_out);
  fputs("const pattern *packed_patterns = &packed_pattern_arr[0];\n", packed_out);

  printf("successes: %d\n"
    "invalids: %d\n"
    "bad_dims: %d\n"
    "bad_rules: %d\n"
    "rle_bytes_used: %d\n"
    "packed_bytes_used: %d\n"
    "both_bytes: %d\n\n"
    "packed as percent of rle: %d\n",
    successes, invalids, bad_dims, bad_rules, rle_bytes_used, packed_bytes_used, both_bytes,
    100 * packed_bytes_used / MAX(rle_bytes_used, 1)
  );

  fflush(rle_out);
  fflush(packed_out);

  fclose(rle_out);
  fclose(packed_out);
}
