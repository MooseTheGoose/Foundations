#include "foundations_lexer.hpp"
#include "foundations_parser.hpp"
#include <stdio.h>
#include <string>
using std::string;

void read_file(const char * fname, string * contents) {
  FILE * fp = fopen(fname, "r");
  const unsigned int BUFSZ = 2048;
  char buf[BUFSZ];
  int nbytes = 1;

  while(nbytes > 0) {
    nbytes = fread(buf, 1, BUFSZ, fp);
    contents -> append(buf, nbytes);
  }
}

int main(int argc, char * argv[]) {
  string fcontents = "";
  read_file(argv[1], &fcontents);
  Lexer * lxr = new_lexer(fcontents.data());
  lxr -> lex();

  if(lxr -> status >= 0) {
    lxr -> print_tokens();
  }

  Parser * parser = new_parser(lxr);
  parser -> parse();
  if(parser -> status >= 0) {
    parser -> root -> print_tree(0);
  }
  free_parser(parser);
  return 0;
}
