#include <sys/defs.h>
#include <sys/string.h>

int strcmp(char *s,char *t){
	while(*s==*t)
	{
		if(*s=='\0')
			return 0;
		s++;
		t++;
	}
	return *s-*t;
}

void strcpy(char *string2, char *string1){

	while(*string1)
	{
		*string2=*string1;
		string1++;
		string2++;
	}
	*string2='\0';
}

int strlen(const char *string)
{
	int length=0;
	while(*string)
	{
		length++;
		string++;
	}
	return length;
}

char* strcat(char *string1, char *string2)
{
	while(*string1!='\0')
	{
		string1++;
	}
	while(*string2!='\0')
	{
		*string1=*string2;
		string1++;
		string2++;
	}
	*string1='\0';
	return string1;
}

char* substring(char* string, int index)
{
	int i = 0;
	while(*string)
	{	
		if(i>index)
		{
			break;
		}
		string++;
		i++;
	}
	return string;
}

uint64_t octal_to_binary(const char* octal)
{
	int oct=0;
	while(*octal)
	{
		oct = (*octal-'0')+(oct*10);
		octal++;
	}
	int octal_dict[8] = {0,1,10,11,100,101,111};
	uint64_t bin=0;
	int i=1;
	int temp=0;
	while(oct!=0)
	{
		temp = oct%10;
		bin += octal_dict[temp]*i;
		i *= 1000;
		oct /= 10;
	}
	return bin;
}

//checks if string1 starts with string2
int starts_with(char* string1, char* string2)
{
	int count = 0;
	while(*string1==*string2 && *string1!='\0')
	{
		count++;
		string1++;
		string2++;
	}
	return count-1;
}

void resetString(char *str){
	*str = '\0';
}