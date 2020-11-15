#ifndef FOUNDATIONS_LEXER_HPP
#define FOUNDATIONS_LEXER_HPP

#include <stdint.h>
#include <vector>
using std::vector;

/*
 *  NOTE: Lexer accepts only ASCII text.
 *        This will obviously change in the future when
 *        I rewrite it in Foundations, but the Foundations
 *        compiler written in C++ will only accept ASCII text
 *        just to get this done and over with.
 */

enum TokenType {
  TT_TERM = 0, TT_NEWLINE, TT_IDENTIFIER,
  TT_OPERATOR, TT_LBRACKET, TT_RBRACKET,
  TT_STRING, TT_INTEGER, TT_FLOATING,
  TT_KEYWORD, TT_STATEMENT_TERM
};

enum TokenOperatorType {
  TO_PLUS = 0, TO_MINUS
};

enum TokenBracketType {
  TB_CURLY = 0, TB_SQUARE, TB_PARANTHESES
};

enum TokenKeyword {
  TKW_INT = 0
};

struct Token {
  enum TokenType type;
  int lino;
  int chno;
  union {
    char * identifier;
    char * strliteral;
    uint64_t intval;
    double floatval;
    enum TokenBracketType bracket;
    enum TokenOperatorType op;
    enum TokenKeyword keyword;
  } value;
};

struct Lexer {
  const char * src;
  size_t src_index;
  size_t src_len;
  vector<Token> * tokens;
  int curr_lino;
  int curr_chno;

  const char * current_byte_ptr();
  int32_t eatchar();
  int32_t peeknext();

  int32_t lex();
  int32_t lex_lbracket();
  int32_t lex_rbracket();
  int32_t lex_number();
  int32_t lex_identifier();
  int32_t lex_statement_term();
  int32_t lex_string();
  int32_t lex_keyword();
  int32_t lex_operator();
};

extern const char * const LINE_SEPARATOR;
extern const char * const STATEMENT_TERMINATOR;
extern const char * TOKEN_TYPE_NAMES[];
extern const char * OPERATOR_STRINGS[];
extern const char * LBRACKET_STRINGS[];
extern const char * RBRACKET_STRINGS[];
extern const char * KEYWORD_STRINGS[];

Lexer * new_lexer(const char * bytes);
void free_lexer(Lexer * lexer);

#endif