#include "string.h"

int StrCmpN(const char *str1, const char *str2, uint32 size)
{
	while (size > 0 && *str1 && *str1 == *str2) {
		size--, str1++, str2++;
    }

	if (size == 0){
        return 0;
    }
    else {
		return (int) ((unsigned char) *str1 - (unsigned char) *str2);
    }
}

int StrLenN(const char *str, uint32 size)
{
    int n;
	for (n = 0; size > 0 && *str != '\0'; str++, size--) {
		n++;
    }

	return n;
}

/*int StrCmp(const char *str1, const char *str2)
{
    while (*str1 && *str1 == *str2) {
        str1++, str2++;
    }

    return (int) ((unsigned char) *p - (unsigned char) *q);
}*/

/*char* strncpy(char *s, const char *t, int n)
{
  char *os;
  
  os = s;
  while(n-- > 0 && (*s++ = *t++) != 0)
    ;
  while(n-- > 0)
    *s++ = 0;
  return os;
}

void strnflip(char *s, size_t size) {
	size_t i;
	char *t;
	char temp;
	t = s + size - 1;
	for (i = 0; i < size/2; i++, s++, t--) {
		temp = *t;
		*t = *s;
		*s = temp;
	}
}*/