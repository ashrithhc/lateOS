#include <sys/idt.h>
#include <sys/kprintf.h>
#include <sys/process.h>
static int count = 0,sec=0;
//char* reg = (char*)0xB8F9E;
//uint16_t PIT_reload_value = 1193;
//static char *reg = (char*)0xB8F9E;
void timer_handler(){
    __asm__ volatile("cli");
	outportb(0x20,0x20);
	count++;
	if(count == 18){
       schedule();
		sec++;
		int temp = sec;
		char *reg = (char*)0xffffffff800B8F9E;
		while(temp>0)
		{
			*reg = '0'+temp%10; 
			temp = temp/10;
			reg-=2;			
		}
		count = 0;
        for(int i=0;i<MAX;i++){
            if(taskQueue[i].state == HANG && taskQueue[i].time >0){
                taskQueue[i].time--;
            }
            if(taskQueue[i].time == 0 && taskQueue[i].state == HANG){
                taskQueue[i].state = RUNNING;
            }
        }
	}
}
