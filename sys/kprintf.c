#include <sys/kprintf.h>
#include "stdarg.h"

static int usedSpace=0;

void kprintf(const char *fmt, ...)
{
	const char *temp1; register char *temp2;
/*
	temp2 = (char *)0xb8000;	

	for(int i=0; i<81; i++){
		*temp2 = 'a';
		temp2 += 2;
	}

	return;*/

	va_list valist;
	va_start(valist, fmt);

	temp2 = (char *)0xb8000;

	for(int move = 0; move < usedSpace; move++){
		temp2+=2;
	}

	int width = 0;
	for(temp1 = fmt, width=0; *temp1; temp1+=1, temp2+=2, width += 1) {
		if(*temp1 == '\n'){
			for(int scount = 0; scount < (80-width); scount++){
				*temp2 = ' ';
				temp2 += 2;
				usedSpace += 1;
			}
		}
		else if(*temp1 != '%') *temp2 = *temp1;
		else {
			temp1 += 1;
			if(*temp1 == 'c') *temp2 = va_arg(valist, int);
			else if(*temp1 == 'd'){
				int input = va_arg(valist, int);
				int a, i=0; char intStr[10];
				while(input > 0){
					a = input % 10;
					intStr[i++] = (char)a + '0';
					input = input/10;
				}
				intStr[i] = '\0';
				i--;

				char printStr[10];
				int k=0;
				while(i>=0){
					printStr[k++] = intStr[i--];
				}
				printStr[k] = '\0';

				for(i=0; printStr[i]!='\0'; i++){
					*temp2 = printStr[i];
					temp2 += 2;
					width += 1;
					usedSpace += 1;
				}
			}
			else if(*temp1 == 's'){
				char *input = va_arg(valist, char*);
				while(*input){
					*temp2 = *input;
					temp2 += 2;
					width += 1;
					input += 1;
					usedSpace += 1;
				}
			}
			else if(*temp1 == 'x'){
				unsigned int input= va_arg(valist, int);
				int a, i=0;
				char str[1024];
				while(input > 0){
					a = input%16;
					input = input/16;
					if(a<10) str[i++] = (char)(a+48);
					else if(a==10) str[i++] = 'a';
					else if(a==11) str[i++] = 'b';
					else if(a==12) str[i++] = 'c';
					else if(a==13) str[i++] = 'd';
					else if(a==14) str[i++] = 'e';
					else  str[i++] = 'f';
				}
				i--;

				char revStr[1024]; int k=0;
				while(i>=0){
					revStr[k++] = str[i--];
				}
				revStr[k] = '\0';

				for(i=0; revStr[i] != '\0'; i++){
					*temp2 = revStr[i];
					temp2 += 2;
					width += 1;
					usedSpace += 1;
				}
			}
			else if(*temp1 == 'p'){
                                unsigned int input= va_arg(valist, int);
                                int a, i=0;
                                char str[1024];
                                while(input > 0){
                                        a = input%16;
                                        input = input/16;
                                        if(a<10) str[i++] = (char)(a+48);
                                        else if(a==10) str[i++] = 'a';
                                        else if(a==11) str[i++] = 'b';
                                        else if(a==12) str[i++] = 'c';
                                        else if(a==13) str[i++] = 'd';
                                        else if(a==14) str[i++] = 'e';
                                        else  str[i++] = 'f';
                                }
                                i--;

                                char revStr[1024]; int k=0;
                                while(i>=0){
                                        revStr[k++] = str[i--];
                                }
                                revStr[k] = '\0';

				*temp2='0'; temp2 += 2;
				*temp2='x'; temp2 += 2;
                                for(i=0; revStr[i] != '\0'; i++){
                                        *temp2 = revStr[i];
                                        temp2 += 2;
					width += 1;
					usedSpace += 1;
                                }
                        }			

			else *temp2 = *temp1;
		}
	}

	va_end(valist);

	usedSpace += (80 - usedSpace%80);
}
