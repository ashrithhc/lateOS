#include <stdlib.h>

static int *setENVvalue, iter; static char **getENVvalue; static char* argv[20], *env[100];
static int globalENV = 0;

int checkforENV(){
  if (iter < globalENV) return globalENV;
  return 0;
}

void initEnvironment(){
  globalENV = checkforENV();
  __asm__ __volatile__ ("movq %%rsp, %0; movq %%rsp, %1" : "=m"(setENVvalue), "=m"(getENVvalue) : :"rsp");
  getENVvalue = getENVvalue + 2;
  for(iter = 0; iter < *(setENVvalue + 2); iter++){
    argv[iter] = (*(getENVvalue++));  
  }
  globalENV++;
  iter=0;
  getENVvalue++;

  while( (*getENVvalue) != NULL){
    env[iter++] = (*(getENVvalue++));
  }
  globalENV--;
  pushenvs(env);
  int retVal = main( *(setENVvalue + 2), argv, env);
  exit(retVal);
}

void _start(void) {
  initEnvironment();
}
