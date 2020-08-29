#include "lexer.hpp"
#include <stdio.h>
#include <stdlib.h>

string test_string = "\xE0\x00\x00";

int main()
{
  lexer lxr = lexer(test_string);

  printf("%X\r\n", lxr.eatchar());
  printf("%d\r\n", lxr.cur_chno);

  return 0;
}
