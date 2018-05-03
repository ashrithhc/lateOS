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
void print_int(int i){
	int j = 1000000000;
	int lead_zero = 1;
	if(i==0){
		put_to_screen('0');
	}
	if(i<0){
		put_to_screen('-');
		i = i* -1;
	}
	while(j!=0){
		if(lead_zero == 1 && (i/j == 0)){
			j = j/10;
			continue;
		}
		if(lead_zero == 1 && (i/j != 0)){
			lead_zero = 0;	
		}
		put_to_screen('0'+(i/j));
		i = i % j;
		j = j/10;
	}
}
void print_pointer(unsigned long i){
	//	put_to_screen('w');	
	char a[32];
        int l=0;
        while(i!=0){
                int k = i%16;
                if(k<10){
                        a[l++] = '0'+k;
                }   
                else{
                        a[l++] = 'A'+(k-10);
                }   
                i = i/16;
        }   
        for(int k=l-1;k>=0;k--){
                put_to_screen(a[k]);
        }   	
}
void print_hex(int i){
	char a[11];
	int l=0;
	while(i!=0){
		int k = i%16;
		if(k<=9){
			a[l++] = '0'+k;
		}
		else{
			a[l++] = 'A'+(k-10);
		}
		i = i/16;
	}
	for(int k=l-1;k>=0;k--){
		put_to_screen(a[k]);	
	}

}
void kprintf(const char *fmt, ...)
{
	const char *temp1;// register char *temp2;
	va_list valist;
	va_start(valist, fmt);

	// temp2 = (char *)(0xffffffff80000000+0xb8000);

	for (temp1 = fmt; *temp1; temp1+=1){
	// while(*temp1!='\0'){
		if(*temp1 == '%'){
			switch(*(temp1+1)){
				case 'd':
					print_int(va_arg(valist,int));
					temp1 = temp1+2;
					break;
				case 'c':
					put_to_screen(va_arg(valist,int));
					temp1 = temp1+2;
					break;
				case 's':
					;
					char* a;
					a = va_arg(valist,char *);
					while(*a!='\0'){
						put_to_screen(*a);
						a++;
					}
					temp1+=2;
					break;
				case 'x':
					print_hex(va_arg(valist,int));
					temp1 = temp1+2;
					break;
				case 'p':
					put_to_screen('0');
					put_to_screen('x');
					unsigned long p  = va_arg(valist,unsigned long);	
					//print_hex(p);
					print_pointer(p);
					temp1+=2;
					break;
				default:
					break;	
			}
		}
		else if(*temp1 == '\\'){
			switch(*(temp1+1)){
				case 'n':
					put_to_screen('\n');
					temp1+=2;
					break;
				case 'r':
					put_to_screen('\r');
					temp1+=2;
					break;
			}
		}
		else{
			put_to_screen(*temp1);
			temp1++;
		}
	}
}

