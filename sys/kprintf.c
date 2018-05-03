//#include <sys/kprintf.h>
#include <stdarg.h>
static char *p_reg = (char*) (0xffffffff80000000 + 0xb8000);
static int h_offset=0,v_offset=0;

void kprintf(const char* fm, ...);
void print_int(int i);
void put_to_screen(char a);
void scroll();

void clrscr()
{
    p_reg = (char*) (0xffffffff80000000 + 0xb8000);
    for(int i=0;i<25;i++){
        for(int j=0; j<80; j++){
            *p_reg = ' ';
            *(p_reg-1) = 0x07;
            p_reg+=2;
        }
    }
    p_reg = (char*) (0xffffffff80000000 + 0xb8000);
    h_offset=0;v_offset=0;
}
void scroll(){
	char* p  = (char*)(0xffffffff80000000 + 0xb8000);
	for(int i=0;i<24;i++){
		for(int j=0;j<160;j=j+2){
			int prev = (i*160)+j;
			int next = (i+1)*160 + j;
			*(p+prev) = *(p+next);
		}
	}
	p_reg = (char*)((0xffffffff80000000 + 0xb8000));
	p_reg = p_reg+(160*23);
	for(int i=0;i<160;i=i+2){
		*(p_reg+i) = '\0';
	}
	p_reg = (char*)((0xffffffff80000000 + 0xb8000))+(160*23);
}
void backspace(){
    if(h_offset == 0){
        return;
    }
    h_offset--;
    p_reg-=2;
    put_to_screen(' ');
    h_offset--;
    p_reg-=2;
}
void put_to_screen(char a){
	if(a == '\n'){
		p_reg =(char*)((0xffffffff80000000 + 0xb8000));
		int k = 80*(v_offset+1);
		for(int i=0;i<k;i++){
			p_reg+=2;
		}
		if(v_offset == 23){
			scroll();	
		}
		else{
			v_offset++;
		}
		h_offset = 0;
		return;
	}
	else if(a == '\r'){
		p_reg = (char*)((0xffffffff80000000 + 0xb8000));
		int k = 80*(v_offset);
		for(int i=0;i<k;i++){
			p_reg+=2;
		}
		h_offset=0;
	}
	else{
		*p_reg = (char)a;
		p_reg+=2;
	}
	if(h_offset == 79){
		if(v_offset == 23){
			//scroll
			scroll();
			p_reg = (char*)(0xffffffff80000000 + 0xb8000);
			p_reg = p_reg + 160*22;
			h_offset = 0;
			return;
		}
		else{
			v_offset++;
			h_offset = 0;
			return;
		}
	}
	h_offset++;

}

void kprintf(const char *fmt, ...)
{
	const char *temp1;
	va_list valist;
	va_start(valist, fmt);

	for (temp1 = fmt; *temp1; ){
		if((*temp1 == '\\') && (*(temp1+1) == 'n')){
			put_to_screen('\n');
			temp1+=2;
		}
		else if((*temp1 == '\\') && (*(temp1+1) == 'r')){
			put_to_screen('\r');
			temp1+=2;
		}				
		else if(*temp1 != '%'){
			put_to_screen(*temp1);
			temp1++;
		}
		else {
			temp1++;
			if (*(temp1) == 'd'){
				int intVal = va_arg(valist, int);
				int offset = 10*5;
				int zeroes = 1;
				if(intVal == 0) put_to_screen('0');
				if(intVal < 0){
					put_to_screen('-');
					intVal = intVal * -1;
				}
				while(offset != 0){
					int modVal = intVal/offset;
					if((zeroes == 1) && (modVal == 0)){offset = offset/10; continue;}
					else if(zeroes == 1 && (modVal != 0)) zeroes = 0;
					put_to_screen('0' + modVal);
					intVal = intVal % offset;
					offset = offset/10;
				}
				temp1++;
			}
			else if (*(temp1) == 'c'){
				put_to_screen(va_arg(valist, int));
				temp1++;
			}
			else if (*(temp1) == 's'){
				char* allChars;
				allChars = va_arg(valist, char *);
				while(*allChars != '\0'){
					put_to_screen(*allChars);
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
				for(int revIndex = index-1; revIndex >= 0; revIndex--) put_to_screen(outList[revIndex]);
				temp1++;
			}
			else if (*(temp1) == 'p'){
				int index;
				put_to_screen('0');
				put_to_screen('x');
				unsigned long intVal = va_arg(valist, unsigned long);
				char outList[100];
		        for(index = 0; intVal != 0; index++){
	                if(intVal%16 < 10) outList[index] = '0' + intVal%16;
	                else outList[index] = 'A' + (intVal%16 - 10);
	                intVal = intVal/16;
		        }   
		        for(int revIndex = index-1; revIndex >= 0; revIndex--) put_to_screen(outList[revIndex]);
				temp1++;
			}
		}
	}
}

