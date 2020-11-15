#ifndef FOUNDATIONS_UTF_HPP
#define FOUNDATIONS_UTF_HPP

#include <string>
#include <stdint.h>
using std::string;

namespace Foundations {

struct Utf8String {
  string * bytes; 
  size_t utf_length; 
  size_t byte_index; 
  size_t utf_index;

  const char * current_byte();

  int32_t eatchar();
  int32_t peeknext();
  int32_t pukechar();
  int32_t peekprev();
};

Utf8String * new_utf8_string(const char * bytes, size_t *err_byteno);
void free_utf8_string(Utf8String * str);

}

#endif

