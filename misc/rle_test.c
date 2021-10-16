#include <ctype.h>
#include <inttypes.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "rules.h"
#include "rle.h"

uint8_t normal_s = 0;
uint8_t normal_b = 0;

char *read_file(char *fname) {
  FILE *f = fopen(fname, "rb");
  if (f == NULL) {
    perror(strerror(errno));
    exit(1);
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
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
    "    .x = %ul,\n"
    "    .y = %ul,\n"
    "  }",
    r.name,
    r.creator,
    r.birth_rules,
    r.stay_alive_rules,
    r.x,
    r.y
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

int main(int argc, char **argv) {
  rule r;
  char *fname = argv[1];
  printf("parsing file %s\n", fname);
  char *file = read_file(fname);
  char *string = file;
  printf("Input file: %s\n", string);

  header_token t = scan_header(string);
  int header_lines_parsed = 0;
  while (t.type != INVALID) {
    header_lines_parsed++;
    printf("parsed header line %d\n", header_lines_parsed);
    switch (t.type) {
      case NAME:
        r.name = t.name;
        break;
      case AUTHOR:
        r.creator = t.author;
        break;
      case XY_RULE:
        printf("RULE\n");
        r.x = t.x;
        r.y = t.y;
        r.stay_alive_rules = t.s;
        r.birth_rules = t.b;
        break;
      case INVALID:
        perror("Coldn't parse file");
        exit(1);
    }
    string = t.next;
    t = scan_header(string);
  }

  fprint_rule(stdout, r);

  free(file);
}
