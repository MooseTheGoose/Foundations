#ifndef FOUNDATIONS_UTILS_HPP
#define FOUNDATIONS_UTILS_HPP

#include <stdint.h>

int is_prefix(const char * prefix, const char * str);
int get_prefix_index(const char * prefixes[], const char *str);
int streq_nocase(const char * str1, const char * str2);
int streq_nocase_index(const char * strv[], const char * stri);
int is_identifier_prefix(int32_t unichar);
int is_space(int32_t unichar);
int is_digit(int32_t unichar);
int is_alpha(int32_t unichar);
int is_xdigit(int32_t unichar);
int32_t conv_xdigit(int32_t xdigit);

#endif