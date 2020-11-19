#include "foundations_lexer.hpp"
#include "foundations_utils.hpp"
#include <string.h>
#include <ctype.h>
#include <string>

using std::string;

const char * const LINE_SEPARATOR = "\n";
const char * const STATEMENT_TERMINATOR = ";";

const char * TOKEN_TYPE_NAMES[] = {
  "TT_TERM", "TT_NEWLINE", "TT_IDENTIFIER",
  "TT_OPERATOR", "TT_LBRACKET", "TT_RBRACKET",
  "TT_STRING", "TT_INTEGER", "TT_FLOATING",
  "TT_KEYWORD", "TT_STATEMENT_TERM"
};

const char * LBRACKET_STRINGS[] = {
  "{", "[", "(", 0
};

const char * RBRACKET_STRINGS[] = {
  "}", "]", ")", 0
};

const char * OPERATOR_STRINGS[] = {
  "<<=", ">>=", "+=", "-=", 
  "*=", "/=", "<=", ">=", 
  "==", "~=", "<<", ">>",
  "&=", "^=", "|=", "%=",
  "!>",
  "*", "/", "+", "-", 
  "=", "~", "|", "&", "^",
  "!", "%", ".", ",", 
  "?", ":", "<", ">", 
  0
}; 

const char * KEYWORD_STRINGS[] = {
  "int", 0
};

Lexer * new_lexer(const char * bytes) {
  Lexer * lxr = new Lexer;
  lxr -> tokens = new vector<Token>;
  lxr -> curr_lino = 1;
  lxr -> curr_chno = 1;
  lxr -> src_index = 0;
  lxr -> src_len = strlen(bytes);
  char * newsrc = new char[lxr -> src_len + 1];
  memcpy(newsrc, bytes, lxr -> src_len + 1);
  lxr -> src = newsrc;
  lxr -> status = 0;
  return lxr;
}

void free_lexer(Lexer * lexer) {
  delete lexer -> tokens;
  delete lexer;
  delete[] lexer -> src;
}

void Lexer::print_tokens() {
  Token * curr_token = this -> tokens -> data();
  while(curr_token -> type != TT_TERM) {
    Token t = *curr_token;
    printf("Type: %s\n", TOKEN_TYPE_NAMES[t.type]);
    printf("Line No.: %d\n", t.lino);
    printf("Char No.: %d\n", t.chno);

    switch(t.type) {
      case TT_IDENTIFIER:
        printf("Identifier: %s\n", t.value.identifier);
        break;
      case TT_INTEGER:
        printf("Value: %llu\n", t.value.intval);
        break;
      case TT_FLOATING:
        printf("Value: %lf\n", t.value.floatval);
        break;
      case TT_STRING:
        printf("String: %s\n", t.value.strliteral);
        break;
      case TT_KEYWORD:
        printf("Keyword: %s\n", KEYWORD_STRINGS[t.value.keyword]);
        break;
      case TT_LBRACKET:
        printf("Bracket: %s\n", LBRACKET_STRINGS[t.value.bracket]);
        break;
      case TT_RBRACKET:
        printf("Bracket: %s\n", RBRACKET_STRINGS[t.value.bracket]);
        break;
      case TT_OPERATOR:
        printf("Operator: %s\n", OPERATOR_STRINGS[t.value.op]);
        break;
    }

    curr_token += 1;
    printf("\n");
  }  
}

const char * Lexer::current_byte_ptr() {
  return this -> src + this -> src_index;
}

int32_t Lexer::eatchar() {
  if(this -> src_index > this -> src_len)
    return -0x81;

  int32_t eat = this -> src[this -> src_index];
  if(eat >= 0) {
    if(is_prefix(LINE_SEPARATOR, this -> current_byte_ptr())) {
      Token tok;

      tok.type = TT_NEWLINE;
      tok.lino = this -> curr_lino;
      tok.chno = this -> curr_chno;
      this -> tokens -> push_back(tok);

      this -> src += strlen(LINE_SEPARATOR);
      this -> curr_lino += 1;
      this -> curr_chno = 1;
    } else {
      this -> src_index += 1;
      this -> curr_chno += 1;
    }
  }
  return eat;
}

int32_t Lexer::peeknext() {
  if(this -> src_index > this -> src_len)
    return -0x81;

  return this -> src[this -> src_index];
}

void Lexer::lex() {
  int32_t curr_char = this -> peeknext();
  Token term;

  while(curr_char > 0) {
    while(is_space(curr_char)) {
      this -> eatchar();
      curr_char = this -> peeknext();
    }

    if(is_prefix(STATEMENT_TERMINATOR, this -> current_byte_ptr())) {
      curr_char = this -> lex_statement_term();
    } else if (get_prefix_index(OPERATOR_STRINGS, this -> current_byte_ptr()) >= 0) {
      curr_char = this -> lex_operator();
    } else if(get_prefix_index(LBRACKET_STRINGS, this -> current_byte_ptr()) >= 0) {
      curr_char = this -> lex_lbracket();
    } else if (get_prefix_index(RBRACKET_STRINGS, this -> current_byte_ptr()) >= 0) {
      curr_char = this -> lex_rbracket();
    } else if (is_prefix("\"", this -> current_byte_ptr()) || is_prefix("'", this -> current_byte_ptr())) {
      curr_char = this -> lex_string();
    } else if (is_identifier_prefix(curr_char)) {
      curr_char = this -> lex_identifier();
    } else if (is_digit(curr_char)) {
      curr_char = this -> lex_number();
    } else if(curr_char < 0) {
      fprintf(stderr, "ERROR at %d:%d\n", this -> curr_lino, this -> curr_chno);
      fprintf(stderr, "Previous error or unknown character. Code %d\n", curr_char);
      this -> status = -1;
      this -> eatchar();
      curr_char = this -> peeknext();
    } else if(curr_char) {
      fprintf(stderr, "Error at %d:%d\n", this -> curr_lino, this -> curr_chno);
      fprintf(stderr, "Bad token prefix\n");
      this -> status = -1;
      this -> eatchar();
      curr_char = this -> peeknext();
    }
  }

  term.type = TT_TERM;
  term.lino = this -> curr_lino;
  term.chno = this -> curr_chno;
  this -> tokens -> push_back(term);
}

int32_t Lexer::lex_operator() {
  Token tok;
  const char * before = this -> current_byte_ptr();

  tok.type = TT_OPERATOR;
  tok.lino = this -> curr_lino;
  tok.chno = this -> curr_chno;
  tok.value.op = (enum TokenOperatorType)get_prefix_index(OPERATOR_STRINGS, before);
  this -> tokens -> push_back(tok);

  const char * operator_str = OPERATOR_STRINGS[tok.value.op];
  while(this -> current_byte_ptr() < before + strlen(operator_str))
    this -> eatchar();

  return this -> peeknext();
}

int32_t Lexer::lex_statement_term() {
  Token tok;
  const char * before = this -> current_byte_ptr();

  tok.type = TT_STATEMENT_TERM;
  tok.lino = this -> curr_lino;
  tok.chno = this -> curr_chno;
  this -> tokens -> push_back(tok);

  while(this -> current_byte_ptr() < before + strlen(STATEMENT_TERMINATOR))
    this -> eatchar();

  return this -> peeknext();
}

int32_t Lexer::lex_lbracket() {
  Token tok;
  const char * before = this -> current_byte_ptr();

  tok.type = TT_LBRACKET;
  tok.lino = this -> curr_lino;
  tok.chno = this -> curr_chno;
  tok.value.bracket = (enum TokenBracketType)get_prefix_index(LBRACKET_STRINGS, before);
  this -> tokens -> push_back(tok);

  const char * bracket_str = LBRACKET_STRINGS[tok.value.bracket];
  while(this -> current_byte_ptr() < before + strlen(bracket_str))
    this -> eatchar();

  return this -> peeknext();
}

int32_t Lexer::lex_rbracket() {
  Token tok;
  const char * before = this -> current_byte_ptr();

  tok.type = TT_RBRACKET;
  tok.lino = this -> curr_lino;
  tok.chno = this -> curr_chno;
  tok.value.bracket = (enum TokenBracketType)get_prefix_index(RBRACKET_STRINGS, before);
  this -> tokens -> push_back(tok);

  const char * bracket_str = RBRACKET_STRINGS[tok.value.bracket];
  while(this -> current_byte_ptr() < before + strlen(bracket_str))
    this -> eatchar();

  return this -> peeknext();
}

int32_t Lexer::lex_string() {
  Token tok;
  string literal = "";

  tok.type = TT_STRING;
  tok.lino = this -> curr_lino;
  tok.chno = this -> curr_chno;

  int32_t quote = this -> eatchar();
  int32_t curr_char = this -> peeknext();
  while(curr_char != quote && curr_char > 0) {
    if(curr_char == '\\') {
      this -> eatchar();
      curr_char = this -> peeknext();
      switch(curr_char) {
        case 'r':
          literal.push_back('\r');
          break;
        case 'n':
          literal.push_back('\n');
          break;
        case 't':
          literal.push_back('\t');
          break;
        case '\'':
        case '"':
          literal.push_back(curr_char);
          break;
        default:
          fprintf(stderr, "ERROR at %d:%d\n", this -> curr_lino, this -> curr_chno);
          fprintf(stderr, "Unrecognized escape sequence\n");
          this -> status = -1;
          while(curr_char != quote && curr_char > 0) {
            this -> eatchar();
            curr_char = this -> peeknext();
          }
          this -> eatchar();
          return this -> peeknext();
      }
    } else if(curr_char > 0) {
      if(is_prefix(LINE_SEPARATOR, this -> current_byte_ptr())) {
        fprintf(stderr, "ERROR at %d:%d\n", this -> curr_lino, this -> curr_chno);
        fprintf(stderr, "Unexpected line separator in string literal\n");
        return -1;
      }
      literal.push_back(curr_char);
    }
    this -> eatchar();
    curr_char = this -> peeknext();
  }

  if(curr_char == '\0') {
    fprintf(stderr, "ERROR at %d:%d\n", this -> curr_lino, this -> curr_chno);
    fprintf(stderr, "Unexpected end to string literal\n");
    return -1;
  }

  char * newliteral = new char[literal.size() + 1];
  memcpy(newliteral, literal.data(), literal.size() + 1);
  tok.value.strliteral = newliteral;
  this -> tokens -> push_back(tok);

  this -> eatchar();
  return this -> peeknext();
}

int32_t Lexer::lex_identifier() {
  Token tok;
  const char * identifier = this -> current_byte_ptr();
  size_t prev_index = this -> src_index;
  int32_t curr_char = this -> peeknext();

  tok.type = TT_IDENTIFIER;
  tok.lino = this -> curr_lino;
  tok.chno = this -> curr_chno;

  while(is_identifier_prefix(curr_char) || is_digit(curr_char)) {
    this -> eatchar();
    curr_char = this -> peeknext();
  }

  size_t identifier_size = this -> src_index - prev_index;
  char * literal = new char[identifier_size + 1];

  memcpy(literal, identifier, identifier_size);
  literal[identifier_size] = '\0';

  
  if(streq_nocase_index(KEYWORD_STRINGS, literal) >= 0) {
    tok.type = TT_KEYWORD;
    tok.value.keyword = (enum TokenKeyword)streq_nocase_index(KEYWORD_STRINGS, literal);
    delete[] literal;
  } else { 
    tok.value.identifier = literal;  
  }
  this -> tokens -> push_back(tok);

  return curr_char;
}

int32_t Lexer::lex_number() {
  Token tok;
  int32_t curr_char = this -> peeknext();
  uint64_t intval = 0;
  double floatval;
  int base = 10;

  tok.type = TT_INTEGER;
  tok.lino = this -> curr_lino;
  tok.chno = this -> curr_chno;

  if(curr_char == '0') {
    this -> eatchar();
    curr_char = this -> peeknext();

    if(tolower(curr_char) == 'x') {
      base = 16;
      this -> eatchar();
      curr_char = this -> peeknext();
    } else if(is_digit(curr_char)) {
      base = 8;
      intval = curr_char - '0';
      if(intval >= base) {
        fprintf(stderr, "ERROR at %d:%d\n", this -> curr_lino, this -> curr_chno);
        fprintf(stderr, "Invalid digit '%c' for base %d\n", curr_char, base);
        this -> status = -1;
        while(is_identifier_prefix(curr_char) || is_digit(curr_char)) {
          this -> eatchar();
          curr_char = this -> peeknext();
        }
        return curr_char;
      }
      this -> eatchar();
      curr_char = this -> peeknext();
    }
  }

  while(is_identifier_prefix(curr_char) || is_digit(curr_char)) {
    int32_t hexval = conv_xdigit(curr_char);
    if(hexval < 0 || hexval >= base) {
      fprintf(stderr, "ERROR at %d:%d\n", this -> curr_lino, this -> curr_chno);
      fprintf(stderr, "Invalid digit '%c' for base %d\n", curr_char, base);
      this -> status = -1;
      while(is_identifier_prefix(curr_char) || is_digit(curr_char)) {
        this -> eatchar();
        curr_char = this -> peeknext();
      }
      return curr_char;
    }
    uint64_t new_intval = intval * base;
    if(new_intval / base < intval || (new_intval + hexval) < intval) {
      fprintf(stderr, "WARNING at %d:%d\n", this -> curr_lino, this -> curr_chno);
      fprintf(stderr, "Integer literal overflow\n");
      this -> status = -1;
      while(is_identifier_prefix(curr_char) || is_digit(curr_char)) {
        this -> eatchar();
        curr_char = this -> peeknext();
      }
      return curr_char;
    }
    this -> eatchar();
    curr_char = this -> peeknext();
    new_intval += hexval;
    intval = new_intval;
  }
  tok.value.intval = intval;
  if(curr_char == '.') {
    tok.value.floatval = (double)tok.value.intval;
    tok.type = TT_FLOATING;
    double fractional_part = 0.0;
    double curr_factor = 1.0 / base;
    this -> eatchar();
    curr_char = this -> peeknext();

    while(is_identifier_prefix(curr_char) || is_digit(curr_char)) {
      int32_t hexval = conv_xdigit(curr_char);
      if(hexval < 0 || hexval >= base) {
        fprintf(stderr, "ERROR at %d:%d\n", this -> curr_lino, this -> curr_chno);
        fprintf(stderr, "Invalid digit '%c' for base %d\n", curr_char, base);
        this -> status = -1;
        while(is_identifier_prefix(curr_char) || is_digit(curr_char)) {
          this -> eatchar();
          curr_char = this -> peeknext();
        }
        return curr_char;
      }
      this -> eatchar();
      curr_char = this -> peeknext();
      fractional_part += curr_factor * hexval;
      curr_factor /= base;
    }
    tok.value.floatval += fractional_part;
  }
  this -> tokens -> push_back(tok);

  return curr_char;
}