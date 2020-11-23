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
 *
 *  TODO:  Make it so that tokens are dynamically allocated
 *         rather than statically allocated.
 */

enum TokenType {
  /* Stage 1 token types */
  TT_TERM = 0, TT_NEWLINE, TT_IDENTIFIER,
  TT_OPERATOR, TT_LBRACKET, TT_RBRACKET,
  TT_STRING, TT_INTEGER, TT_FLOATING,
  TT_KEYWORD, TT_STATEMENT_TERM,
  TT_TREE
};

enum TokenOperatorType {
  TO_RSHIFT_ASGN, TO_LSHIFT_ASGN, TO_PLUS_ASGN, TO_MINUS_ASGN,
  TO_TIMES_ASGN, TO_DIV_ASGN, TO_LEQ, TO_GEQ,  
  TO_EQ, TO_NEQ, TO_LSHIFT, TO_RSHIFT,
  TO_BWAND_ASGN, TO_BWXOR_ASGN, TO_BWOR_ASGN, TO_MOD_ASGN,
  TO_ADDR,
  TO_TIMES, TO_DIV, TO_PLUS, TO_MINUS, 
  TO_ASGN, TO_BWNOT, TO_BWOR, TO_BWAND, TO_BWXOR,
  TO_DEREF, TO_MODULUS, TO_DOT, TO_COMMA,
  TO_TERN_Q, TO_TERN_C, TO_LT, TO_GT
};

enum TokenBracketType {
  TB_CURLY = 0, TB_SQUARE, TB_PARANTHESES
};

enum TokenKeyword {
  TKW_INT = 0, TKW_S8, TKW_S16, TKW_S32, TKW_S64,
  TKW_UINT, TKW_U8, TKW_U16, TKW_U32, TKW_U64,
  TKW_XOR, TKW_AND, TKW_NOT, TKW_OR
};

struct DerivationTree;

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
    DerivationTree * tree;
  } value;
};

struct Lexer {
  char * src;
  size_t src_index;
  size_t src_len;
  vector<Token> * tokens;
  vector<char *> * string_heap;
  int curr_lino;
  int curr_chno;
  int status;

  const char * current_byte_ptr();
  int32_t eatchar();
  int32_t peeknext();

  void lex_stage1();
  void lex();

  int32_t lex_lbracket();
  int32_t lex_rbracket();
  int32_t lex_number();
  int32_t lex_identifier();
  int32_t lex_statement_term();
  int32_t lex_string();
  int32_t lex_keyword();
  int32_t lex_operator();

  void print_tokens();
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

void print_tokens(const Token * tokens);

#endif