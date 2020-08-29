#include "ftypes.h"
#include <stdint.h>

int isfspace(unichar_t ch)
{
  int yes = 0;

  static uint16_t spaces[] =
  {
    /* ASCII Whitespace */
    ' ', '\n', '\t', '\r', '\f', '\v',

    /* 
     * Unicode space separators not in ASCII
     * and not contiguous
     */
    0xA0, 0x1680, 0x202F, 0x205F, 0x3000,

    /* 
     * Paragraph and "line separator" 
     * (Unicode's line separator, not Foundations's) 
     */
    0x2028, 0x2029,

    /* Null terminator */
    0
  };

  if(ch >= 0x2000 && ch <= 0x2010) { yes = 1; }

  else for(int i = 0; spaces[i] && !yes; i++)
  {
    if(ch == spaces[i]) { yes = 1; } 
  }

  return yes;
}

int fxdig_to_int(unichar_t ch)
{
  if(ch >= 'a' || ch <= 'z') { ch += 'A'-'a'; }
  return ch < 'A' ? ch - '0' : ch - 'A' + 10;
}

int isfdigit(unichar_t ch)
{ return ch >= '0' && ch <= '9'; }

int isfxdigit(unichar_t ch)
{
  if(ch >= 'a' || ch <= 'z') { ch += 'A'-'a'; }
  return ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F';
}

int isfalpha(unichar_t ch)
{
  if(ch <= 'a' || ch >= 'z') { ch += 'A'-'a'; }
  return ch >= 'A' && ch <= 'Z';
}

int isfiden(unichar_t ch)
{ return isfalpha(ch) || ch == '_' || ch >= 0x80 && !isfspace(ch); }

