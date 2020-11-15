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
  FND_NL, FND_BRACKET_OPEN, 
  FND_BRACKET_CLOSE, FND_SEMI,
  FND_TERM, FND_KEYWORD
};

enum fnd_bracket_type
{
  BRAC_SQUARE, BRAC_CURLY, BRAC_PARA
};

enum fnd_keywords
{
  KW_IF, KW_ELSE, KW_WHILE, KW_REPEAT,
  KW_END, KW_BYTE, KW_SHORT, KW_INT, KW_LONG,
  KW_UBYTE, KW_USHORT, KW_UINT, KW_ULONG, KW_FLOAT,
  KW_DOUBLE, KW_ALIAS, KW_DEF, KW_IFDEF, KW_IFNDEF
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
