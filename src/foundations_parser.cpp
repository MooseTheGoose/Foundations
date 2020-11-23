#include "foundations_parser.hpp"
#include "foundations_utils.hpp"
#include <string.h>

const char * DERIVATION_TYPE_STRINGS[] = {
  "DT_ROOT", "DT_IDENTIFIER", "DT_STRING", "DT_OPERATOR",
  "DT_FLOATING", "DT_INTEGER", "DT_KEYWORD",
  "DT_LAMBDA", "DT_ARRAY", "DT_STRUCT"
};

const char * DERIVATION_OPERATOR_STRINGS[] = {
  "<<=", ">>=", "+=", "-=", 
  "*=", "/=", "<=", ">=", 
  "==", "~=", "<<", ">>",
  "&=", "^=", "|=", "%=",
  "!>",
  "*", "/", "+", "-", 
  "=", "~", "|", "&", "^",
  "!", "%", ".", ",", 
  "?", ":", "<", ">",
  "::", ":=",
  "not", "and", "xor", "or",
  "()", 0
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

Parser * new_parser(Lexer * lexer) {
  Parser * parsr = new Parser;
  parsr -> root = 0;
  parsr -> lexer = lexer;
  parsr -> tokenptr = 0;
  parsr -> status = 0;
  return parsr;
}

void free_parser(Parser * parser) {
  size_t i = 0;
  free_tree(parser -> root);
  free_lexer(parser -> lexer);
  delete parser;
}

void DerivationTree::print_tree(size_t indent) {
  char * spaces = new char[indent + 1];
  const char *str;
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
      printf(" operator='%s'", DERIVATION_OPERATOR_STRINGS[this -> value.op]);
      break;
    case DT_FLOATING:
      printf(" value='%lf'", this -> value.floatval);
      break;
    case DT_INTEGER:
      printf(" value='%llu'", this -> value.intval);
      break;
    case DT_KEYWORD:
	  printf(" value='%s'", KEYWORD_STRINGS[this -> value.keyword]);
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

void free_tree(DerivationTree * tree) {
  DerivationTree * child = tree -> children;
 
  while(child) {
    free_tree(child);
    child = child -> next;
  }
  delete tree;
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
    Token t = tokens[start];
    switch(tokens[start].type) {
      case TT_INTEGER:
        expr -> type = DT_INTEGER;
        expr -> value.intval = t.value.intval;
        break;
      case TT_FLOATING:
        expr -> type = DT_FLOATING;
        expr -> value.floatval = t.value.floatval;
        break;
      case TT_IDENTIFIER:
        expr -> type = DT_IDENTIFIER;
        expr -> value.identifier = t.value.identifier;
        break;
      case TT_KEYWORD:
        expr -> type = DT_KEYWORD;
        expr -> value.keyword = t.value.keyword;
      break;
      case TT_STRING:
        expr -> type = DT_STRING;
        expr -> value.strliteral = t.value.strliteral;
        break;
      case TT_TREE:
        expr = t.value.tree;
        expr -> mark = 1;
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

	/* 
     * Level 15 operations (Commas).
     *
     * This has to do with multiple return values,
     * so you won't see this for a while...
     */

	/* Level 14 operations (Binary definitions) */
    for(size_t index = start + 1; index < end - 1; index++) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && curr_token -> value.op == TO_TERN_C) {
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
		DerivationTree * lchild = parse_expr(tokens, start, index);
		DerivationTree * rchild;
		expr -> children = lchild;
		if(tokens[index + 1].type == TT_OPERATOR) {
		  rchild = new DerivationTree;
		  rchild -> type = DT_OPERATOR;
          rchild -> value.op = (DerivationOperatorType)tokens[index + 1].value.op;
		  rchild -> lino = tokens[index + 1].lino;
		  rchild -> chno = tokens[index + 1].chno;
		  rchild -> next = 0;
		  rchild -> children = parse_expr(tokens, index + 2, end);
        } else {
          rchild = parse_expr(tokens, index + 1, end);
        }
		lchild -> next = rchild;
		return expr;
      }      
    }


	/* Level 13 operations (Assignment) */

	for(size_t index = start + 1; index < end - 1; index++) {
      const Token * curr_token = tokens + index;
	  const int BINARY_OPERATORS[] = {
	  	TO_RSHIFT_ASGN, TO_LSHIFT_ASGN, TO_PLUS_ASGN, TO_MINUS_ASGN,
	  	TO_TIMES_ASGN, TO_DIV_ASGN, TO_MOD_ASGN, TO_BWAND_ASGN,
	  	TO_BWXOR_ASGN, TO_BWOR_ASGN, TO_ASGN, NULL_OPERATOR
	  };

      if(curr_token -> type == TT_OPERATOR && get_operator_index(BINARY_OPERATORS, curr_token -> value.op) >= 0) {
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }  		
	}

	/* Level 12 operations (Logical OR) */
    for(size_t index = end - 2; index > start; index--) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_KEYWORD && curr_token -> value.keyword == TKW_OR) {
        expr -> value.op = DO_LOG_OR;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

	/* Level 11 operations (Logical XOR) */
    for(size_t index = end - 2; index > start; index--) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_KEYWORD && curr_token -> value.keyword == TKW_XOR) {
        expr -> value.op = DO_LOG_XOR;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

	/* Level 10 operations (Logical AND) */
    for(size_t index = end - 2; index > start; index--) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_KEYWORD && curr_token -> value.keyword == TKW_AND) {
        expr -> value.op = DO_LOG_AND;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

	/* Level 9 operations (Logical NOT) */
    for(size_t index = end - 1; index >= start && index != 0; index--) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_KEYWORD && curr_token -> value.keyword == TKW_NOT) {
        expr -> value.op = DO_LOG_NOT;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * child = parse_expr(tokens, index + 1, end);
        expr -> children = child;
        return expr;
      }      
    }

    /* Level 8 operations (Bitwise OR) */
    for(size_t index = end - 2; index > start; index--) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && curr_token -> value.op == TO_BWOR) {
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 7 operations (Bitwise XOR) */
    for(size_t index = end - 2; index > start; index--) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && curr_token -> value.op == TO_BWXOR) {
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 6 operations (Bitwise AND) */
    for(size_t index = end - 2; index > start; index--) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && curr_token -> value.op == TO_BWAND) {
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 5 operations (Logical comparisons, including ~= and ==)*/
    for(size_t index = end - 2; index > start; index--) {
      const int BINARY_OPERATORS[] = {
        TO_LT, TO_LEQ, TO_GT, TO_GEQ, TO_EQ, TO_NEQ, NULL_OPERATOR
      };
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && get_operator_index(BINARY_OPERATORS, curr_token -> value.op) >= 0) {
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 4 operations (Bitwise shift operations) */
    for(size_t index = end - 2; index > start; index--) {
      const int BINARY_OPERATORS[] = {
        TO_RSHIFT, TO_LSHIFT, NULL_OPERATOR
      };
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && get_operator_index(BINARY_OPERATORS, curr_token -> value.op) >= 0) {
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 3 operations (Binary + and -) */
    for(size_t index = end - 2; index > start; index--) {
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
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }      
    }

    /* Level 2 operations (*, /, %) */
    for(size_t index = end-2; index > start; index--) {
      const int BINARY_OPERATORS[] = {
        TO_TIMES, TO_DIV, TO_MODULUS, NULL_OPERATOR
      };
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && get_operator_index(BINARY_OPERATORS, curr_token -> value.op) >= 0) {
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * rchild = parse_expr(tokens, index + 1, end);
        DerivationTree * lchild = parse_expr(tokens, start, index);
        expr -> children = lchild;
        lchild -> next = rchild;
        return expr;
      }
    }

    /* Level 1 operations. (Address + Deref, Unary + and -, ~) */
    for(size_t index = start; index < end-1; index++) {
      const int UNARY_OPERATORS[] = {
        TO_ADDR, TO_DEREF, TO_PLUS, TO_MINUS, TO_BWNOT, NULL_OPERATOR
      };
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TT_OPERATOR && get_operator_index(UNARY_OPERATORS, curr_token -> value.op) >= 0) {
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
        expr -> lino = curr_token -> lino;
        expr -> chno = curr_token -> chno;
        DerivationTree * child = parse_expr(tokens, index + 1, end);
        expr -> children = child;
        return expr;
      }
    }

    /* Level 0 operations (dot operator, lambda calls, complex literals). */
    for(size_t index = end-2; index >= start && index != 0; index--) {
      const Token * curr_token = tokens + index;
      if(curr_token -> type == TO_DOT) {
        expr -> value.op = (DerivationOperatorType)curr_token -> value.op;
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

DerivationTree * Parser::fetch_next_expr(vector<Token> * stor) {
  size_t tokptr = this -> tokenptr;
  const Token * tokens = lexer -> tokens -> data();
  const Token * curr_token = tokens + tokenptr;

  while(curr_token -> type != TT_TERM && curr_token -> type != TT_NEWLINE && curr_token -> type != TT_STATEMENT_TERM) {
	switch(curr_token -> type) {
      case TT_LBRACKET:
        this -> tokenptr = tokptr;
        switch(curr_token -> value.bracket) {
          case TB_PARANTHESES:
            DerivationTree * para = parse_para();
            Token tok;
            tok.type = TT_TREE;
            tok.lino = para -> lino;
            tok.chno = para -> chno;
            tok.value.tree = para;
            stor -> push_back(tok);
            break;
        }
        if(tokens[this -> tokenptr].type != TT_TERM)
          this -> tokenptr += 1;
        tokptr = this -> tokenptr;
        break;
	  default:
		stor -> push_back(*curr_token);
		tokptr += 1;
		break;
	}
    curr_token = tokens + tokptr;
  }

  this -> tokenptr = tokptr;
  if(stor -> size() > 0) {
    DerivationTree * new_node = this -> parse_expr(stor -> data(), 0, stor -> size());
    sweep(stor);
    stor -> clear();
    return new_node;
  }
  return 0;
}

void sweep(vector<Token> * tokens) {
  for(size_t i = 0; i < tokens -> size(); i++) {
    Token * storage_token = tokens -> data() + i;
    if(storage_token -> type == TT_TREE && storage_token -> value.tree -> mark == 0) {
      free_tree(storage_token -> value.tree);
    }
  }
}

DerivationTree * Parser::parse_para() {
  const Token * tokens = this -> lexer -> tokens -> data();
  size_t tokptr = this -> tokenptr + 1;
  vector<Token> stor = vector<Token>();
  DerivationTree * para = new DerivationTree;

  para -> type = DT_OPERATOR;
  para -> lino = tokens[tokptr-1].lino;
  para -> chno = tokens[tokptr-1].chno;
  para -> mark = 0;
  para -> value.op = DO_PARA_EXPR; 
  para -> next = 0;
  para -> children = 0;

  /*
   *  Remember to recursively call this for nested paras
   *  and to send error for TT_TERM. 
   */
  while((tokens[tokptr].type != TT_RBRACKET || tokens[tokptr].value.bracket != TB_PARANTHESES)
         && tokens[tokptr].type != TT_TERM) {
    if(tokens[tokptr].type != TT_NEWLINE) {
      if(tokens[tokptr].type == TT_LBRACKET) {
        switch(tokens[tokptr].value.bracket) {
          case TB_PARANTHESES:
            this -> tokenptr = tokptr;
            DerivationTree * new_expr = this -> parse_para();
            tokptr = this -> tokenptr;
            Token tok;
            tok.type = TT_TREE;
            tok.lino = new_expr -> lino;
            tok.chno = new_expr -> chno;
            tok.value.tree = new_expr;
            stor.push_back(tok);
            break;
        }
      } else {
        stor.push_back(tokens[tokptr]);
      }
    }
    tokptr += 1;
  }

  if(tokens[tokptr].type == TT_TERM) {
    fprintf(stderr, "ERROR on %d:%d\n", para -> lino, para -> chno);
    fprintf(stderr, "No matching right parantheses\n");
    status = -1;
  } else if(stor.size() > 0) {
    para -> children = this -> parse_expr(stor.data(), 0, stor.size());
  }

  sweep(&stor);
  this -> tokenptr = tokptr;

  return para;
}

void Parser::parse() {
  vector<Token> expr_storage = vector<Token>();
  const Token * tokens = lexer -> tokens -> data();

  this -> root = new DerivationTree;
  this -> root -> lino = tokens -> lino;
  this -> root -> chno = tokens -> chno;
  this -> root -> type = DT_ROOT;
  this -> root -> next = 0;
  this -> root -> children = 0;
  DerivationTree ** curr_child = & this -> root -> children;
  while(tokens[this -> tokenptr].type != TT_TERM) {
	DerivationTree * new_node = this -> fetch_next_expr(&expr_storage);
    if(new_node) {
	  *curr_child = new_node;
	  curr_child = &new_node -> next;
    }
    if(tokens[this -> tokenptr].type != TT_TERM)
      this -> tokenptr += 1;
  }
}