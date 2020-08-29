#ifndef FTYPES_H
#define FTYPES_H

#include <stdint.h>

typedef int32_t unichar_t; 

int isfspace(unichar_t ch);
int isfdigit(unichar_t ch);
int isfxdigit(unichar_t ch);
int fxdig_to_int(unichar_t ch);
int isfiden(unichar_t ch);
int isfalpha(unichar_t ch);

#endif

