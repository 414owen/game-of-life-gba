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

void fprint_rule(FILE *out, rule r) {
  fprintf(out, "  {\n"
    "    .name = \"%s\",\n"
    "    .creator = \"%s\",\n"
    "    .birth_rules = %" PRIu8 ",\n"
    "    .stay_alive_rules = %" PRIu8 ",\n"
    "    .width = %d,\n"
    "    .height = %d,\n"
    "  }",
    r.name,
    r.creator,
    r.birth_rules,
    r.stay_alive_rules,
    r.width,
    r.height
  );
}

void fprint_rules(FILE *out, int rule_amt, rule *rules) {
  fprintf(out, "int rule_amt = %d;\n"
    "rule rules[] = {\n",
    rule_amt);
  for (int i = 0; i < rule_amt; i++) {
    fprint_rule(out, rules[i]);
    fputs(",\n", out);
  }
  fputs("};\n", out);
}

void run_file(char *fname) {
  rule r;
  printf("parsing file %s\n", fname);
  size_t file_len = 0;
  char *file = read_file(fname, &file_len);
  char *cursor = file;

  header_token t = scan_header(cursor);
  int header_lines_parsed = 0;
  while (t.type != INVALID) {
    header_lines_parsed++;
    switch (t.type) {
      case NAME:
        r.name = t.name;
        break;
      case AUTHOR:
        r.creator = t.author;
        break;
      case XY_RULE:
        // r.x = t.x;
        // r.y = t.y;
        r.stay_alive_rules = t.s;
        r.birth_rules = t.b;
        break;
      case INVALID:
        perror("Coldn't parse file");
        exit(1);
    }
    cursor = t.next;
    t = scan_header(cursor);
  }

  int width = 0;
  int height = 0;

  get_dims(cursor, file + file_len, &width, &height);

  fprint_rule(stdout, r);
  // printf("width: %d, height: %d\n", width, height);

  free(file);
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
  while ((dir = readdir(d)) != NULL) {
    if (dir->d_type != DT_REG) continue;
    size_t len = strlen(dir->d_name);
    char *fname = malloc(len + dir_len + 1);
    strcpy(fname, dir_prefix);
    strcpy(fname + dir_len, dir->d_name);
    fname[len + dir_len] = '\0';
    run_file(fname);
    free(fname);
  }
  closedir(d);
}
