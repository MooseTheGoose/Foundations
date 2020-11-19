#include "foundations_lexer.hpp"
#include "foundations_parser.hpp"
#include <stdio.h>

int main(int argc, char * argv[]) {
  Lexer * lxr = new_lexer(argv[1]);
  lxr -> lex();

  if(lxr -> status >= 0) {
    lxr -> print_tokens();
  }

  Parser parser;
  parser.status = 0;

  DerivationTree *expr = parser.parse_expr(lxr -> tokens -> data(), 0, lxr -> tokens -> size() - 1);
  if(parser.status >= 0) {
    expr -> print_tree(0);
  }

  return 0;
}
