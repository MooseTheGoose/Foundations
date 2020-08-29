#ifndef LEXER_HPP
#define LEXER_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include <string.h>
#include "ftypes.h"

using std::string;
using std::vector;

#define LINE_SEPARATOR "\n"

enum fnd_token_type
{
  FND_IDEN, FND_OP, FND_NUM,
  FND_NL, FND_BRACKET, FND_SEMI
};

struct fnd_token
{
  int32_t type;
  int32_t lino;
  int32_t chno;
  int32_t aux;  
  union
  {
    uint64_t bits;
  };
};

struct lexer
{
  vector<fnd_token> lang_toks;
  string source;
  int32_t cur_lino;
  int32_t cur_chno;
  size_t index;

  lexer(const string &src);
  unichar_t peekchar();
  unichar_t eatchar();
  void lex();
};


#endif
