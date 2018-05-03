#include <sys/idt.h>
#include <sys/kprintf.h>

static char *RREG = (char*)0xffffffff800B8F90;
static char *PREG = (char*)0xffffffff800B8F8E;
static int caps=0, controlValue=0;
static char code_map[58][2] =
     {
       {   0,0   } ,
       {   0,0   } ,
       { '1','!' } ,
       { '2','@' } ,
       { '3','#' } ,
       { '4','$' } ,
       { '5','%' } ,
       { '6','^' } ,
       { '7','&' } ,
       { '8','*' } ,
       { '9','(' } ,
       { '0',')' } ,
       { '-','_' } ,
       { '=','+' } ,
       {   8,8   } ,
       {   9,9   } ,
       { 'q','Q' } ,
       { 'w','W' } ,
       { 'e','E' } ,
       { 'r','R' } ,
       { 't','T' } ,
       { 'y','Y' } ,
       { 'u','U' } ,
       { 'i','I' } ,
       { 'o','O' } ,
       { 'p','P' } ,
       { '[','{' } ,
       { ']','}' } ,
       { '^','M' } ,
       { ' ','^' } ,
       { 'a','A' } ,
       { 's','S' } ,
       { 'd','D' } ,
       { 'f','F' } ,
       { 'g','G' } ,
       { 'h','H' } ,
       { 'j','J' } ,
       { 'k','K' } ,
       { 'l','L' } ,
       { ';',':' } ,
       {  39,34  } ,
       { '`','~' } ,
       {   0,0   } ,
       { '\\','|'} ,
       { 'z','Z' } ,
       { 'x','X' } ,
       { 'c','C' } ,
       { 'v','V' } ,
       { 'b','B' } ,
       { 'n','N' } ,
       { 'm','M' } ,
       { ',','<' } ,
       { '.','>' } ,
       { '/','?' } ,
       {   0,0   } ,
       {   0,0   } ,
       {   0,0   } ,
       { ' ',' ' } ,
   };

static inline uint8_t inb(uint64_t port)
{
       uint8_t retVal;
       __asm__ __volatile__ ("inb %1, %0" : "=a"(retVal) : "Nd"(port));
       return retVal;
}

void setPREG(int code, int index){
       *PREG = code_map[code][index];
}

void setRREG(int code, int index){
       *RREG = code_map[code][index];
}

void kb()
{
	char character;
	int code = inb(0x60);
	if(code==28){
              setPREG(code, 0);
              setRREG(code, 1);
	}
	else if(code==29){
		controlValue = 1;
              setPREG(code, controlValue);
		*PREG = code_map[code][controlValue];
	}
	else if ((code == 42) || (code == 54)) caps = 1;
	else if(code > 0){
		if(controlValue==1)
		{	
			controlValue=0;
			caps=1;	
		}	
		else setPREG(29, controlValue);
              setRREG(code, caps);
		caps = 0;
	}	
	outportb(0x20,0x20);	
}