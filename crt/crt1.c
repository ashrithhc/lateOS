#include <stdlib.h>

void _start(void) {
  int *setENVvalue, iter; char **getENVvalue; char* argv[20], *env[100];
  __asm__ __volatile__ ("movq %%rsp, %0; movq %%rsp, %1" : "=m"(setENVvalue), "=m"(getENVvalue) : :"rsp");
  getENVvalue = getENVvalue + 2;
  for(iter = 0; iter < *(setENVvalue + 2); iter++){
    argv[iter] = (*(getENVvalue++));	
  }
  iter=0;
  getENVvalue++;

  while( (*getENVvalue) != NULL){
    env[iter++] = (*(getENVvalue++));
  }
  pushenvs(env);

  int retVal = main( *(setENVvalue + 2), argv, env);
  exit(retVal);
}
