hash_token scan_header(const char *YYCURSOR) {
  char *YYMARKER;
  char *name, *ename;
  char *author, *eauthor;
  char *x, *y, *ex, *ey;
  char *s, *b, *es, *eb;
  /*!stags:re2c format = "const char *@@; "; */
  hash_token token;
  token.type = INVALID;
loop:
/*!re2c
    re2c:define:YYCTYPE = char;
    re2c:yyfill:enable  = 0;
    re2c:flags:tags = 1;
    re2c:tags:expression = "@@";

    // ignore comments
    // [#][Cc].*         { goto loop; }

    sp = [ \t];
    osp = [\t ]*;
    nl = [\r\n]+;
    str = .*;
    integer = [-]*[0-9]+;

    * {
      token.type = INVALID;
      goto ret;
    }

    [#] [Cc] .* [\n] { printf("skipping comment\n"); goto loop; }

    [#] [N] osp @name str @ename osp nl {
      token.type = NAME;
      token.name = mk_string(name, ename);
      goto ret;
    }

    [#] [O] osp @author str @eauthor osp nl {
      token.type = AUTHOR;
      token.author = mk_string(author, eauthor);
      goto ret;
    }

    [x] osp [=] osp @x integer @ex  osp [,] osp
    [y] osp [=] osp @y integer @ey osp nl {
      token.type = XY_RULE;
      token.x = parse_int(x);
      token.y = parse_int(y);
      token.s = normal_s;
      token.b = normal_b;
      goto ret;
    }

    [x] osp [=] osp @x integer @ex  osp [,] osp
    [y] osp [=] osp @y integer @ey osp [,] osp
    'rule' osp [=] osp @s [0-9]+ @es [/] @b [0-9]+ @eb osp nl {
      token.type = XY_RULE;
      token.x = parse_int(x);
      token.y = parse_int(y);
      token.s = parse_rule_bitset(s, es);
      token.b = parse_rule_bitset(b, eb);
      goto ret;
    }

    [x] osp [=] osp @x integer @ex  osp [,] osp
    [y] osp [=] osp @y integer @ey osp [,] osp
    'rule' osp [=] osp [B] @b [0-9]+ @eb [/] [S] @s [0-9]+ @es osp nl {
      token.type = XY_RULE;
      token.x = parse_int(x);
      token.y = parse_int(y);
      token.s = parse_rule_bitset(s, es);
      token.b = parse_rule_bitset(b, eb);
      goto ret;
    }

*/

ret:
  token.next = YYCURSOR;
  return token;
}


