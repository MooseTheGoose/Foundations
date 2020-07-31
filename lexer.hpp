#ifndef LEXER_HPP
#define LEXER_HPP

#include <stdint.h>
#include <vector>
#include <string>

using std::string;
using std::vector;

enum fnd_token_type
{
  FND_IDEN, FND_PRIM_TYPE, FND_OP,
  FND_STORAGE, FND_IMM, FND_TERM,
  FND_IGNORE
};

struct fnd_token
{
  int type;
  int lino;
  int chno;
  int aux;  
  union
  {
  };
};

struct lexer
{
  string full_text
  vector<string> lines;
  
  vector<preproc_token> preproc_toks;
  vector<fnd_token> lang_toks;

  void lex();  
};

#endif