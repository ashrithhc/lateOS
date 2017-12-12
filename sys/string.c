#include <stdio.h>
#include <string.h>

char str[10];
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

int strcmp(const char *str1, const char *str2){
	while(*str1 && *str2 && *str1 == *str2){	
		str1++;
		str2++;
	}
	return *str1 - *str2;
}

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

int strlen(const char *str){
	int len = 0;
	while(*str++) len++;
	return len;
}


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


char * index ( char *str, char chr){
	while(*str){
		if(*str == chr){
			return (char *) str;
		str++;
	}
	return NULL;
}


void bzero(void *str, unsigned n){
	char *buf = str;
	while(n != 0 ){
		*buf++ = 0;
		n--;
	}
}



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

char *int_to_str( int num){
	char temp;
	int index = 0, j = 0;
	char *t = str;
	int n = 5;
	while(n!= 0){
		*t++ = 0;
		n--;
	}
	if(num == 0 ){
		str[index] = '0';
	}
	while(num > 0){
		str[index++] = '0' + (num%10);
		num/=10;
	}
	
	for(j = 0; j < index/2; j++){
		temp = str[j];
		str[j] = str[index-1-j];
		str[index-1-j] = temp;
	}
	
	return str;
}

	

