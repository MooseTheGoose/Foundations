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

    /* ... */
    /* Level 5 operations */

    /* Level 4 operations */
    for(size_t index = start; index < end; index++) {
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
    for(size_t index = start; index < end; index++) {
      const int BINARY_OPERATORS[] = {
        TO_PLUS, TO_MINUS, NULL_OPERATOR
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

    /* Level 2 operations */
    for(size_t index = start; index < end; index++) {
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
    for(size_t index = end-1; index >= start; index--) {
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
    for(size_t index = start; index < end; index++) {
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