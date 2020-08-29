#include "lexer.hpp"
#include "ftypes.h"
#include <stdio.h>

lexer::lexer(const string& src)
{
  lang_toks = vector<fnd_token>();
  source = src;
  index = 0;
  cur_chno = 1;
  cur_lino = 1;
};

unichar_t lexer::peekchar()
{
  size_t local_index = this->index;
  unichar_t retchar = (unsigned char)this->source[local_index];

  if(retchar & 0x80)
  {
    int n_more_bytes = 0;
    unichar_t cur_char = retchar;
    unsigned char prefix = cur_char << 1;

    while(prefix & 0x80)
    { n_more_bytes++; prefix <<= 1; }

    if(n_more_bytes < 1 || n_more_bytes > 3)
    { return -1; }

    retchar = prefix >> (n_more_bytes + 1);
    local_index++;   

    unichar_t low_bound = 0x04;
    unichar_t high_bound = 0x400;
    for(int i = 0; i < n_more_bytes; i++)
    {
      cur_char = this->source[local_index++];
      if((cur_char & 0xC0) != 0x80) { return -1; }
      retchar <<= 6;
      retchar |= (cur_char & 0x3F);

      low_bound <<= 5;
    }

    if(retchar < low_bound || retchar > 0x10FFFF) { return -1; }
  }

  return retchar;
}

unichar_t lexer::eatchar()
{
  if(!this->source.compare(this->index, strlen(LINE_SEPARATOR), LINE_SEPARATOR))
  {
    fnd_token nltok;
    nltok.type = FND_NL;
    nltok.lino = this->cur_lino++;
    nltok.chno = ++this->cur_chno;
    nltok.aux = 0;

    this->cur_chno = 0;
    this->lang_toks.push_back(nltok); 
  }

  unichar_t cur_char = this->peekchar();

  if(cur_char >= 0)
  {
    this->cur_chno++;

    if(cur_char >= 0x80)
    {
      size_t local_index = this->index;
      while((this->source[++local_index] & 0xC0) == 0x80) {}
      this->index = local_index;
    }
    else 
    { this->index++; }
  }

  return cur_char;
}

void lexer::lex()
{
  while(this->index < this->source.length())
  {
    unichar_t cur_char = this->peekchar();
    while(isfspace(cur_char))
    {
      this->eatchar();
      cur_char = this->peekchar();
    }
  }
}
