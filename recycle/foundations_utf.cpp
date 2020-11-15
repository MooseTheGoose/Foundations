#include "foundations_utf.hpp"

Foundations::Utf8String * Foundations::new_utf8_string(const char *bytes, size_t *err_byteno) {
  Foundations::Utf8String * utf = new Foundations::Utf8String;
  size_t local_utf_length = 0;
  utf -> bytes = new string;
  string * local_str = utf -> bytes;
  size_t local_byte_index = 0;
  unsigned char curr_char;
  
  curr_char = bytes[local_byte_index];
  while(curr_char) {
    local_str -> push_back((char)curr_char);
    local_byte_index += 1;

	if(curr_char >= 0x80) {
      unsigned char prefix = curr_char << 1;
      int32_t val;
      int total_bytes = 1;
      const int32_t low_bounds[3] = {
        0x80, 0x800, 0x10000
      };

      while(prefix & 0x80) {
        total_bytes += 1;
        prefix <<= 1;
      }

      if(total_bytes < 2 || total_bytes > 4) {
        delete local_str;
        delete utf;
        if(err_byteno)
          *err_byteno = local_byte_index - 1;
        return 0;
      }

      prefix >>= total_bytes;
      val = prefix;
      for(int i = 1; i < total_bytes; i += 1) {
        curr_char = bytes[local_byte_index];
        local_byte_index += 1;
        if((curr_char & 0xC0) != 0x80) {
          delete local_str;
          delete utf;
          if(err_byteno)
            *err_byteno = local_byte_index - 1;
          return 0;
        }
        val <<= 6;
        val |= curr_char & 0x3F;
        local_str -> push_back((char)curr_char);
      }

      if(val < low_bounds[total_bytes - 2] || val > 0x10FFFF) {
        delete local_str;
        delete utf;
        if(err_byteno)
          *err_byteno = local_byte_index - 1;
        return 0;
      }
    }

    curr_char = bytes[local_byte_index];
    local_utf_length += 1;
  }

  utf -> byte_index = 0;
  utf -> utf_index = 0;
  utf -> utf_length = local_utf_length;
  return utf;
}

void Foundations::free_utf8_string(Foundations::Utf8String * str) {
  delete str -> bytes;
  delete str;
}

int32_t Foundations::Utf8String::peeknext() {
  if(this -> utf_length < this -> utf_index)
    return -1;

  size_t local_byte_index = this -> byte_index;
  unsigned char * local_bytes = (unsigned char *)this -> bytes -> data();
  int32_t peek = local_bytes[local_byte_index];
    
  local_byte_index += 1;
  if(peek >= 0x80) {
    unsigned char prefix = peek << 1;
    int32_t val;
    int total_bytes = 1;

    while(prefix & 0x80) {
      prefix <<= 1;
      total_bytes += 1;
    }

    prefix >>= total_bytes;
    val = prefix;
    for(int i = 1; i < total_bytes; i += 1) {
      peek = local_bytes[local_byte_index];
      local_byte_index += 1;
      val <<= 6;
      val |= peek & 0x3F;
    }

    peek = val;
  }

  return peek;
}

int32_t Foundations::Utf8String::eatchar() {
  if(this -> utf_length < this -> utf_index)
    return -1;

  int32_t eat = this -> peeknext();
  size_t local_byte_index = this -> byte_index;
  int32_t bounds[4] = { 0, 0x80, 0x800, 0x10000 };

  for(int i = 0; i < 4; i++)
    if(eat >= bounds[i])
      local_byte_index += 1;

  if(local_byte_index != this -> byte_index)
    this -> utf_index += 1;
  this -> byte_index = local_byte_index;

  return eat;
}

int32_t Foundations::Utf8String::peekprev() {
  if(this -> byte_index == 0)
    return -1;

  unsigned char * local_bytes = (unsigned char *)this -> bytes -> data();
  size_t local_byte_index = this -> byte_index;

  while((local_bytes[local_byte_index] & 0xC0) == 0x80)
    local_byte_index -= 1;

  int nbytes = this -> byte_index - local_byte_index + 1;
  this -> utf_index -= 1;
  this -> byte_index -= nbytes;
  int32_t peek = this -> eatchar();
  if(peek < 0) {
    this -> utf_index += 1;
    this -> byte_index += nbytes;
  }
  return peek;
}

int32_t Foundations::Utf8String::pukechar() {
  if(this -> byte_index == 0)
    return -1;

  unsigned char * local_bytes = (unsigned char *)this -> bytes -> data();
  int32_t peek;
  size_t local_byte_index = this -> byte_index;

  while((local_bytes[local_byte_index] & 0xC0) == 0x80)
    local_byte_index -= 1;

  int nbytes = this -> byte_index - local_byte_index + 1;
  this -> utf_index -= 1;
  this -> utf_index -= nbytes;
  return this -> peeknext();
}

const char * Foundations::Utf8String::current_byte() {
  return this -> bytes -> data() + this -> byte_index;
}
