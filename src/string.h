#ifndef STRING_H
#define STRING_H

#include "types.h"

int StringCmp(const char* str1, const char* str2);
int	StringCmpN(const char* str1, const char* str2, uint32 size);
int	StringLenN(const char* str, uint32 size);
char* StringFindChar(const char *str, char c);

/*char* StrCopyN(char *dst, const char *src, uint32 size);
void StrFlipN(char *s, uint32 size);*/

#endif
