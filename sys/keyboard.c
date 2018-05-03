#include <sys/idt.h>
#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/process.h>
#include <sys/terminal.h>

#define True 1
#define False 0

static char *RREG = (char *)0xffffffff800B8F90;
static char *PREG = (char *)0xffffffff800B8F8E;
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


void wokeUP(int index){
       (&(taskQueue[index]))->state = RUNNING;
}

void wake_process(){
    for(int i=0; i<MAX; i++) if((&(taskQueue[i]))->state == SLEEP) {wokeUP(i); break;}
}

void kb()
{
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
		if(controlValue==1){	
			controlValue=0;
			caps=1;	
		}	
		else setPREG(29, controlValue);
              setRREG(code, caps);
		caps = 0;
	}	
	outportb(0x20,0x20);	
}

static int offset = 0;

int getoffset(){
    return offset;
}

void setoffset(int i){
    offset = i;
}


static int strOffset = 0;
int lineCount;
char inpStr[4096];

void write_terminal()
{
       char key_pressed;
       int code = inb(0x60);
       if(code > 58) {
              outportb(0x20, 0x20);
              return;
       }
       if(code == 14){
              if(offset != strOffset){
                     *(inpStr + strOffset - 1) = '\0';
                     strOffset--;
                     backspace();
              }
       }
       else if(code == 28){
              kprintf("\n");
              inpStr[strOffset] = '\n';
              lineCount++;
              strOffset++;
              wake_process();
              outportb(0x20, 0x20);
              return;
       }
       else if(code == 29){
              controlValue = 1;
              kprintf("%c", code_map[code][controlValue]);
              *(inpStr+strOffset) = code_map[code][controlValue];
              strOffset++;
       }
       else if ((code == 42) || (code == 54)) caps=1;
       else if (code > 0){
              if(controlValue==1){      
                     controlValue=0;
                     caps=1;       
              }
              key_pressed = code_map[code][caps];
              caps=0;
              kprintf("%c", key_pressed);
              *(inpStr + strOffset) = key_pressed;
              strOffset++;                 
       }      
       outportb(0x20, 0x20); 
}

void read_input(char* inpString){
    while(True){
       int stringOffset = 0;
       if(lineCount>0){
              for(int strOffset = getoffset(); strOffset<4096; strOffset++){
                     if( inpStr[strOffset] == '\n'){
                            setoffset(strOffset+1);
                            lineCount--;
                            currentTask->state = RUNNING;
                            return;
                     }
                     *(inpString+stringOffset) = inpStr[strOffset];
                     inpStr[strOffset] = '\0';
                     stringOffset++;
              }
        }
        else currentTask->state = SLEEP;
        schedule();
    }
}