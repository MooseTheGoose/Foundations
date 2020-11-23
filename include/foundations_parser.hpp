#ifndef FOUNDATIONS_PARSER_HPP
#define FOUNDATIONS_PARSER_HPP

#include "foundations_lexer.hpp"
#include <string.h>

enum DerivationType {
  DT_ROOT, DT_IDENTIFIER, DT_STRING, DT_OPERATOR,
  DT_FLOATING, DT_INTEGER, DT_KEYWORD,
  DT_LAMBDA, DT_ARRAY, DT_STRUCT
};

enum DerivationOperatorType {
  DO_RSHIFT_ASGN = TO_RSHIFT_ASGN, 
  DO_LSHIFT_ASGN = TO_LSHIFT_ASGN, 
  DO_PLUS_ASGN = TO_PLUS_ASGN, 
  DO_MINUS_ASGN = TO_MINUS_ASGN,
  DO_TIMES_ASGN = TO_TIMES_ASGN, 
  DO_DIV_ASGN = TO_DIV_ASGN, 
  DO_LEQ = TO_LEQ, 
  DO_GEQ = TO_GEQ,  
  DO_EQ = TO_EQ, 
  DO_NEQ = TO_NEQ, 
  DO_LSHIFT = TO_LSHIFT, 
  DO_RSHIFT = TO_RSHIFT,
  DO_BWAND_ASGN = TO_BWAND_ASGN, 
  DO_BWXOR_ASGN = TO_BWXOR_ASGN, 
  DO_BWOR_ASGN = TO_BWOR_ASGN, 
  DO_MOD_ASGN = TO_MOD_ASGN,
  DO_ADDR = TO_ADDR,
  DO_TIMES = TO_TIMES, 
  DO_DIV = TO_DIV, 
  DO_PLUS = TO_PLUS, 
  DO_MINUS = TO_MINUS, 
  DO_ASGN =  TO_ASGN, 
  DO_BWNOT = TO_BWNOT, 
  DO_BWOR = TO_BWOR, 
  DO_BWAND = TO_BWAND, 
  DO_BWXOR = TO_BWXOR,
  DO_DEREF = TO_DEREF, 
  DO_MODULUS = TO_MODULUS, 
  DO_DOT = TO_DOT, 
  DO_COMMA = TO_COMMA,
  DO_TERN_Q = TO_TERN_Q, 
  DO_TERN_C = TO_TERN_C, 
  DO_LT = TO_LT, 
  DO_GT = TO_GT,
  DO_TERN_CONST,
  DO_TERN_VAR,
  DO_LOG_NOT, 
  DO_LOG_AND, 
  DO_LOG_XOR, 
  DO_LOG_OR,
  DO_PARA_EXPR
};

struct DerivationTree {
  enum DerivationType type;
  int lino;
  int chno;
  int mark;
  DerivationTree * children;
  DerivationTree * next;
  union {
    char * identifier;
    char * strliteral;
    uint64_t intval;
    double floatval;
    enum DerivationOperatorType op;
    enum TokenKeyword keyword;
	size_t arraylen;
  } value;

  void print_tree(size_t indent);
};

struct Parser {
  DerivationTree * root;
  Lexer * lexer;
  size_t tokenptr;
  int status;

  void parse();
  DerivationTree * parse_expr(const Token * tokens, size_t start, size_t end);
  DerivationTree * fetch_next_expr(vector<Token> * stor);
  DerivationTree * parse_para();
};

void free_tree(DerivationTree * tree);
Parser * new_parser(Lexer * lexer);
void free_parser(Parser * parser);
void sweep(vector<Token> * tokens);


extern const char * DERIVATION_TYPE_STRINGS[];
extern const char * DERIVATION_OPERATOR_STRINGS[];

#define NULL_OPERATOR (-1)

#endif