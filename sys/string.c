#include <sys/string.h>
#include <sys/defs.h>

const char *strcpy(char *dest, const char *src){
	const char *retStr = dest;
	while (*src) *dest++ = *src++;
	*dest = '\0';
	return retStr;
}

/*char str[10];
const char* strcpy(char *dest, const char *src){
	const char *buffer = dest;
	while(*src){
		*dest = *src;
		src++;
		dest++;
	}
	*dest = '\0';
	return buffer;
}

const char* strncpy(char *dest, const char* src, int count){
	const char *buffer = dest;
	while(count && *src){	
		*dest = *src;
		src++;
		dest++;
		count--;
	}
	*dest = '\0';
	return buffer;
}
*/
int strcmp(const char *str1, const char *str2){
	while(*str1 && *str2 && *str1 == *str2){	
		str1++;
		str2++;
	}
	return *str1 - *str2;
}
/*
int strncmp(const char *str1, const char *str2, int count){
	while(count){
		if(*str1 && *str2 && *str1 == *str2){
			str1++;
			str2++;
		}
		else{
			return *str1 - *str2;
			break;
		}
		count--;
	}
	return 0;
}
*/
int strlen(const char *str){
	int len = 0;
	while(*str++) len++;
	return len;
}
/*

void strcat(char *dest, const char *src){
	while(*dest++);
	while((*dest++ = *src++));
	*dest = '\0';
}


void strncat( char *dest, const char *src,int count){
	if(count){
		while(*++dest);
		while(count && *src){	
			*dest++ = *src++;
			count--;
		}
		*dest = '\0';
	}
}



*/

char *strtok(char *str, const char *delim){
	static char *token = NULL;
	char *str_ptr = NULL;
	int index = 0;
	int str_len = strlen(delim);
	
	if(!str && !token)
		return NULL;
	if(str && !token)
		token = str;

	str_ptr = token;
	while(1){
		for(index = 0; index < str_len; index++ ){
			if(*str_ptr == delim[index]){
				str_ptr++;
				break;
			}
		}
		
		if(index == str_len){
			token = str_ptr;
			break;
		}
	}
	
	if(*token == '\0'){
		token = NULL;
		return token;
	}
	
	while(*token != '\0'){
		for(index = 0; index < str_len; index ++){
			if(*token == delim[index]){
				*token = '\0';
				break;
			}
		}
		token++;

		if(index < str_len) break;
	}
	return str_ptr;
} 

/*	
void memset(void *ptr, int value, uint64_t num){
	int i =0;
	for(; i< num ; i++)
		((char*)ptr)[i] = 0;
}


void *memcpy( void *dest, const void *src, uint64_t n ) {
	unsigned char *buf1 = (unsigned char*) dest;
	const unsigned char *buf2 = (unsigned char*) src;
	if( buf2 < buf1){
		for( buf1 += n, buf2 += n; n--;  ){
			*--buf1 = *--buf2;
		}
	}
	else{
		while(n--)	
			*buf1++ = *buf2++;
	}
	return dest;
}
*/
