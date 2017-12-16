#include <string.h>

const char *strcpy(char *dest, char *src){
	const char *retStr = dest;
	while (*src) *dest++ = *src++;
	*dest = '\0';
	return retStr;
}
