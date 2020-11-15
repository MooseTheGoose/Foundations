#include "foundations_utils.hpp"
#include <ctype.h>

int is_space(int32_t unichar) {
  return isspace(unichar);
}

int is_prefix(const char * prefix, const char * str) {
  while(*prefix == *str && *prefix && *str) {
    prefix += 1;
    str += 1;
  }
  return *prefix == '\0';
}

int get_prefix_index(const char * prefixes[], const char * str) {
  int index = 0;
  while(prefixes[index]) {
    if(is_prefix(prefixes[index], str))
      return index;
    index += 1;
  }
  return -1;
}

int streq_nocase(const char * str1, const char * str2) {
  int nocase_str1 = tolower(*str1);
  int nocase_str2 = tolower(*str2);

  while(nocase_str1 == nocase_str2 && nocase_str1 && nocase_str2) {
    str1 += 1;
    str2 += 1;
    
    nocase_str1 = tolower(*str1);
    nocase_str2 = tolower(*str2);
  }
  return nocase_str1 == nocase_str2;
}

int streq_nocase_index(const char * strv[], const char * stri) {
  int index = 0;
  while(strv[index]) {
    if(streq_nocase(strv[index], stri))
      return index;
    index += 1;
  }
  return -1;
}

int is_alpha(int32_t unichar) {
  unichar = tolower(unichar);
  return unichar >= 'a' && unichar <= 'z';
}

int is_digit(int32_t unichar) {
  return unichar >= '0' && unichar <= '9';
}

int is_xdigit(int32_t unichar) {
  return isxdigit(unichar);
}

int is_identifier_prefix(int32_t unichar) {
  return is_alpha(unichar) || unichar == '_';
}

int32_t conv_xdigit(int32_t unichar) {
  return is_digit(unichar) ? unichar - '0' : toupper(unichar) - 'A' + 10;
}