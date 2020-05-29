#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

enum PreProcTokenType
{
    PP_MACRO, PP_SLCOMMENT, PP_DELIM,
    PP_TOKEN
};

enum FndTokenType
{
    FT_IDEN, FT_PRIM_TYPE, FT_DELIM,
    FT_STORAGE, FT_TERM
};

typedef struct FndPrim
{
    int primtype;
    uint64_t bits;
} FndPrim;

typedef struct PreProcToken
{
    int type;
    union
    {
       
    }
    val;
}
PreProcToken;

typedef struct FndToken
{
    int type;
    union
    {
        char *iden;
        FndPrim primitive;
        int delimiter;
        int storage_class;
    } val;
} FndToken;


#endif