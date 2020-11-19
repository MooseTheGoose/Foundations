#include "foundations_parser.hpp"

const char * DERIVATION_TYPE_STRINGS[] = {
  "DT_IDENTIFIER", "DT_STRING", "DT_OPERATOR",
  "DT_FLOATING", "DT_INTEGER"
};

int get_operator_index(const int ops[], int lookup) {
  int index = 0;
  while(ops[index] != NULL_OPERATOR) {
    if(ops[index] == lookup)
      return index;
    index += 1;
  }
  return -1;
}

int is_value(const Token * token) {
  return token -> type == TT_IDENTIFIER 
         || token -> type == TT_INTEGER
         || token -> type == TT_FLOATING;
}

void DerivationTree::print_tree(size_t indent) {
  char * spaces = new char[indent + 1];
  memset(spaces, ' ', indent);
  spaces[indent] = '\0';
  printf("%s<%s lino='%d' chno='%d'", spaces, DERIVATION_TYPE_STRINGS[this -> type], this -> lino, this -> chno);
  switch(this -> type) {
    case DT_IDENTIFIER:
      printf(" identifier='%s'", this -> value.identifier);
      break;
    case DT_STRING:
      printf(" string='%s'", this -> value.strliteral);
      break;
    case DT_OPERATOR:
      printf(" operator='%s'", OPERATOR_STRINGS[this -> value.op]);
      break;
    case DT_FLOATING:
      printf(" value='%lf'", this -> value.floatval);
      break;
    case DT_INTEGER:
      printf(" value='%llu'", this -> value.intval);
      break;
  }
  printf(">\n");

  DerivationTree * child = this -> children;
  while(child) {
    child -> print_tree(indent + 2);
    child = child -> next;
  }
  printf("%s</%s>\n", spaces, DERIVATION_TYPE_STRINGS[this -> type]);
}

/*
 *  Tokens must not have any new line tokens,
 *  so you'll need a new array of tokens to pass to
 *  the parser. 
 *
 *  NOTE: For certain operations, I put bounds not quite
 *        at the endpoints to simplify implementation for
 *        certain levels. Note levels that have exclusively
 *        unary, binary, or ternary operators, and note operators
 *        which are both binary and unary, but with different
 *        precedences.
 */
DerivationTree * Parser::parse_expr(const Token * tokens, size_t start, size_t end) {
  DerivationTree * expr = new DerivationTree;
  expr -> next = 0;
  expr -> children = 0;

  if(start >= end) {
    fprintf(stderr, "ERROR at %d:%d\n", tokens[start].lino, tokens[start].chno);
    fprintf(stderr, "Expected non-empty expression here\n");
    this -> status = -1;
  } else if(start + 1 == end) {
    expr -> lino = tokens[start].lino;
    expr -> chno = tokens[start].chno;
    switch(tokens[start].type) {
      case TT_INTEGER:
        expr -> type = DT_INTEGER;
        expr -> value.intval = tokens[start].value.intval;
        break;
      case TT_FLOATING:
        expr -> type = DT_FLOATING;
        expr -> value.floatval = tokens[start].value.floatval;
        break;
      case TT_IDENTIFIER:
        expr -> type = DT_IDENTIFIER;
        expr -> value.identifier = tokens[start].value.identifier;
        break;
      default:
        fprintf(stderr, "Error at %d:%d\n", expr -> lino, expr -> chno);
        fprintf(stderr, "Expected valid single-term expression\n");
        this -> status = -1;
        break;
    }
    return expr;
  } else {
    expr -> type = DT_OPERATOR;

	
	/* Level 16 operations (Ternary definitions) */
	/* Level 15 operations (Comma) */
	/* Level 14 operations (Assignment) */
	/* Level 13 operations (Ternary conditional) */

    for(size_t index = end-2; index >= start + 1; index--) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && curr_token -> value.op == TO_TERN_Q) {
		for(size_t colon_index = index + 2; colon_index < end-1; colon_index++) {
          const Token * test_token = tokens + colon_index;
          if(test_token -> type == TT_OPERATOR && test_token -> value.op == TO_TERN_C) {
            expr -> value.op = curr_token -> value.op;
            expr -> lino = curr_token -> lino;
            expr -> chno = curr_token -> chno;
            DerivationTree * rchild = parse_expr(tokens, colon_index + 1, end);
		    DerivationTree * mchild = parse_expr(tokens, index + 1, colon_index);
            DerivationTree * lchild = parse_expr(tokens, start, index);
            expr -> children = lchild;
            lchild -> next = mchild;
            mchild -> next = rchild;
            return expr;
          }
        }
      }      
    }

	/* Level 9, 10, 11, 12 operations (NOT, AND, XOR, OR). */

    /* Level 8 operations */
    for(size_t index = start + 1; index < end-1; index--) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && curr_token -> value.op == TO_BWOR) {
        expr -> value.op = curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 7 operations */
    for(size_t index = start + 1; index < end-1; index++) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && curr_token -> value.op == TO_BWXOR) {
        expr -> value.op = curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 6 operations */
    for(size_t index = start + 1; index < end-1; index++) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && curr_token -> value.op == TO_BWAND) {
        expr -> value.op = curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 5 operations */
    for(size_t index = start + 1; index < end-1; index++) {
      const int BINARY_OPERATORS[] = {
        TO_LT, TO_LEQ, TO_GT, TO_GEQ, TO_EQ, TO_NEQ, NULL_OPERATOR
      };
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && get_operator_index(BINARY_OPERATORS, curr_token -> value.op) >= 0) {
        expr -> value.op = curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 4 operations */
    for(size_t index = start + 1; index < end-1; index++) {
      const int BINARY_OPERATORS[] = {
        TO_RSHIFT, TO_LSHIFT, NULL_OPERATOR
      };
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && get_operator_index(BINARY_OPERATORS, curr_token -> value.op) >= 0) {
        expr -> value.op = curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 3 operations */
    for(size_t index = start + 1; index < end-1; index++) {
      const int BINARY_OPERATORS[] = {
        TO_PLUS, TO_MINUS, NULL_OPERATOR
      };
      const Token * curr_token = tokens + index;
      /* 
       * NOTE: Plus and minus are also unary operators and those unary
       *       operators have higher precedence, so check that the token 
       *       before it is not an operator or that it even exists. Only then
       *       does this operator become binary or throw an error.
       */
      if(curr_token -> type == TT_OPERATOR && get_operator_index(BINARY_OPERATORS, curr_token -> value.op) >= 0
         && is_value(tokens + index - 1)) {
        expr -> value.op = curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 2 operations */
    for(size_t index = start + 1; index < end-1; index++) {
      const int BINARY_OPERATORS[] = {
        TO_TIMES, TO_DIV, TO_MODULUS, NULL_OPERATOR
      };
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && get_operator_index(BINARY_OPERATORS, curr_token -> value.op) >= 0) {
        expr -> value.op = curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }
    }

    /* Level 1 operations. */
    for(size_t index = end-2; index >= start; index--) {
      const int UNARY_OPERATORS[] = {
        TO_ADDR, TO_DEREF, TO_PLUS, TO_MINUS, TO_BWNOT, NULL_OPERATOR
      };
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && get_operator_index(UNARY_OPERATORS, curr_token -> value.op) >= 0) {
        expr -> value.op = curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * child = parse_expr(tokens, index + 1, end);
        expr -> children = child;
        return expr;
      }
    }

    /* Level 0 operations. */
    for(size_t index = start; index < end-1; index++) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TO_DOT) {
        expr -> value.op = curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }
    }

    fprintf(stderr, "ERROR at %d:%d\n", tokens[start].lino, tokens[start].chno);
    fprintf(stderr, "Expected to find operator in expression, but none found\n");
    this -> status = -1;
  }

  return expr;
}