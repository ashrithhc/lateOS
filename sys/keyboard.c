#include <sys/idt.h>
#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/process.h>
#include <sys/terminal.h>

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

static int offset = 0;
// static int caps=0;
// static int controlValue=0;
static int i=0;
// static uint8_t inb(uint64_t port);
int no_lines;
char buf[4096];

void write_terminal()
{
       char key_pressed;
       int c=0;
       //while(1)
       //{
              if(inb(0x60)!=0){
                     c=inb(0x60);
              }
              if(c>0)
              {
                     if(c>58)
                     {
                            outportb(0x20, 0x20);
                            return;
                     }
                     if(c==14)//Backspace
                     {
                if(offset != i) {
                    *(buf + i - 1) = '\0';
                    i--;
                    backspace();
                }
            }
                     else if(c==28)
                     {
                            kprintf("\n");
                buf[i] = '\n';
                no_lines++;
                i++;
                wake_process();
                            outportb(0x20, 0x20);
                            return;
                     }
                     else if(c==29)
                     {
                            controlValue=1;
                            kprintf("%c",code_map[c][controlValue]);
                            *(buf+i) = code_map[c][controlValue];
                            i++;
                     }
                     else if(c==42||c==54)
                     //else if(c==32)
                     {
                            caps=1;
                     }
                     else
                     {
                            /*key_pressed=code_map[c][caps];
                            caps=0;
                            *reg=key_pressed;*/
                            if(controlValue==1)
                            {      
                                   controlValue=0;
                                   caps=1;       
                            }      
                            else
                            {
                                   /*kprintf("%c",code_map[29][controlValue]);
                                   *buf = code_map[29][controlValue];
                                   buf++;*/
                            }
                            key_pressed=code_map[c][caps];
                            caps=0;
                            kprintf("%c",key_pressed);
                            *(buf+i) = key_pressed;
                            i++;                 
                     }      
              }             
       //}
       outportb(0x20,0x20); 
}
void read_input(char* b){
    while(1){
        if(no_lines>0){
            int j=0;
            for(int i = getoffset();i<4096;i++,j++){
                if( buf[i] == '\n'){
                    setoffset(i+1);
                    no_lines--;
                    currentTask->state = RUNNING;
                    return;
                }
                *(b+j) = buf[i];
                buf[i] = '\0';
            }
        } else{
            currentTask->state = SLEEP;
        }
        schedule();
    }
}

void wake_process(){
    for(int i=0;i<MAX;++i){
        if(taskQueue[i].state == SLEEP){
            taskQueue[i].state = RUNNING;
            //      schedule();
            return;
        }
    }
}

int getoffset(){
    return offset;
}
void setoffset(int i){
    offset = i;
}