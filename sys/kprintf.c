#include <stdarg.h>
#define kernbase 0xffffffff80000000
#define videomem 0xb8000
#define maxwidth 80
#define maxheight 24

static int toLeft = 0, numLines = 0;
static char *temp2 = (char*) (kernbase + videomem);

void clearScreen()
{
    temp2 = (char*) (kernbase + videomem);
    for(int i=0; i<=maxheight; i++){
        for(int j=0; j<maxwidth; j++){
            *temp2 = ' ';
            *(temp2-1) = 0x07;
            temp2 += 2;
        }
    }

    temp2 = (char*) (kernbase + videomem);
    toLeft=0; numLines=0;
}

void rollUP(){
	char* temp2  = (char*)(kernbase + videomem);
	for(int i=0; i<maxheight; i++){
		for(int j=0; j<(maxwidth*2); j=j+2){
			int prev = (i*(maxwidth*2))+j;
			int next = (i+1)*(maxwidth*2) + j;
			*(temp2+prev) = *(temp2+next);
		}
	}
	temp2 = (char*)((kernbase + videomem));
	temp2 = temp2 + ((maxwidth*2)*(maxheight-1));
	for(int i=0; i<(maxwidth*2); i=i+2) *(temp2+i) = '\0';
	temp2 = (char*)((kernbase + videomem)) + ((maxwidth*2)*(maxheight-1));
}

void writeToSbush(char ch){
	if(ch == '\n'){
		temp2 =(char *)((kernbase + videomem));
		for(int i=0; i < maxwidth*(numLines + 1); i++) temp2 += 2;
		if(numLines == (maxheight-1)) rollUP();
		else numLines++;
		toLeft = 0;
		return;
	}
	else if(ch == '\r'){
		temp2 = (char *)((kernbase + videomem));
		for(int i=0; i<maxwidth*(numLines); i++) temp2 += 2;
		toLeft=0;
	}
	else{
		*temp2 = (char)ch;
		temp2 += 2;
	}

	if ((toLeft == 79) && (numLines == (maxheight-1))){
		rollUP();
		temp2 = (char*)(kernbase + videomem);
		temp2 = temp2 + (maxwidth*2)*22;
		toLeft = 0;
	}
	else if((toLeft == 79) && (numLines != (maxheight-1))){
		numLines++;
		toLeft = 0;
	}
	else toLeft++;
}

void backspace(){
    if(toLeft == 0) return;
    toLeft--; temp2-=2;
    writeToSbush(' ');
    toLeft--; temp2-=2;
}

void kprintf(const char *fmt, ...)
{
	const char *temp1;
	va_list valist;
	va_start(valist, fmt);

	for (temp1 = fmt; *temp1; ){
		if((*temp1 == '\\') && (*(temp1+1) == 'n')){
			writeToSbush('\n');
			temp1+=2;
		}
		else if((*temp1 == '\\') && (*(temp1+1) == 'r')){
			writeToSbush('\r');
			temp1+=2;
		}				
		else if(*temp1 != '%'){
			writeToSbush(*temp1);
			temp1++;
		}
		else {
			temp1++;
			if (*(temp1) == 'd'){
				int intVal = va_arg(valist, int);
				int offset = 10*5;
				int zeroes = 1;
				if(intVal == 0) writeToSbush('0');
				if(intVal < 0){
					writeToSbush('-');
					intVal = intVal * -1;
				}
				while(offset != 0){
					int modVal = intVal/offset;
					if((zeroes == 1) && (modVal == 0)){offset = offset/10; continue;}
					else if(zeroes == 1 && (modVal != 0)) zeroes = 0;
					writeToSbush('0' + modVal);
					intVal = intVal % offset;
					offset = offset/10;
				}
				temp1++;
			}
			else if (*(temp1) == 'c'){
				writeToSbush(va_arg(valist, int));
				temp1++;
			}
			else if (*(temp1) == 's'){
				char* allChars;
				allChars = va_arg(valist, char *);
				while(*allChars != '\0'){
					writeToSbush(*allChars);
					allChars++;
				}
				temp1++;
			}
			else if (*(temp1) == 'x'){
				int index;
				int intVal = va_arg(valist, int);
				char outList[100];
				for(index = 0; intVal != 0; index++){
					if(intVal%16 <= 9) outList[index] = '0' + intVal%16;
					else outList[index] = 'A' + (intVal%16 - 10);
					intVal = intVal/16;
				}
				for(int revIndex = index-1; revIndex >= 0; revIndex--) writeToSbush(outList[revIndex]);
				temp1++;
			}
			else if (*(temp1) == 'p'){
				int index;
				writeToSbush('0');
				writeToSbush('x');
				unsigned long intVal = va_arg(valist, unsigned long);
				char outList[100];
		        for(index = 0; intVal != 0; index++){
	                if(intVal%16 < 10) outList[index] = '0' + intVal%16;
	                else outList[index] = 'A' + (intVal%16 - 10);
	                intVal = intVal/16;
		        }   
		        for(int revIndex = index-1; revIndex >= 0; revIndex--) writeToSbush(outList[revIndex]);
				temp1++;
			}
		}
	}
}

