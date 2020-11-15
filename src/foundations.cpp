#include "foundations_lexer.hpp"
#include <stdio.h>

int main() {
  Lexer * lxr = new_lexer("  ;iNt intEGER {+}HELL0 _WORLD; 1234.5678 1024 \"The World is Black\\n\"\n; ;; ;\n");
  int status = lxr -> lex();

  if(status >= 0) {
    for(Token t : (*lxr -> tokens)) {
      printf("Type: %s\n", TOKEN_TYPE_NAMES[t.type]);
      printf("Line No.: %d\n", t.lino);
      printf("Char No.: %d\n", t.chno);

      switch(t.type) {
        case TT_IDENTIFIER:
          printf("Identifier: %s\n", t.value.identifier);
          break;
        case TT_INTEGER:
          printf("Value: %llu\n", t.value.intval);
          break;
        case TT_FLOATING:
          printf("Value: %lf\n", t.value.floatval);
          break;
        case TT_STRING:
          printf("String: %s\n", t.value.strliteral);
          break;
        case TT_KEYWORD:
          printf("Keyword: %s\n", KEYWORD_STRINGS[t.value.keyword]);
          break;
        case TT_LBRACKET:
          printf("Bracket: %s\n", LBRACKET_STRINGS[t.value.bracket]);
          break;
        case TT_RBRACKET:
          printf("Bracket: %s\n", RBRACKET_STRINGS[t.value.bracket]);
          break;
        case TT_OPERATOR:
          printf("Operator: %s\n", OPERATOR_STRINGS[t.value.op]);
          break;
      }

      printf("\n");
    }
  }

  return 0;
}
