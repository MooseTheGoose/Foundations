#ifndef FOUNDATIONS_PARSER_HPP
#define FOUNDATIONS_PARSER_HPP

#include "foundations_lexer.hpp"
#include <string.h>

enum DerivationType {
  DT_IDENTIFIER, DT_STRING, DT_OPERATOR,
  DT_FLOATING, DT_INTEGER
};

struct DerivationTree {
  enum DerivationType type;
  int lino;
  int chno;
  DerivationTree * children;
  DerivationTree * next;
  union {
    char * identifier;
    char * strliteral;
    uint64_t intval;
    double floatval;
    enum TokenOperatorType op;
  } value;

  void print_tree(size_t indent);
};

struct Parser {
  DerivationTree * root;
  int status;

  void parse(const Token * tokens);
  DerivationTree * parse_expr(const Token * tokens, size_t start, size_t end);
};

extern const char * DERIVATION_TYPE_STRINGS[];

#define NULL_OPERATOR -1

#endif