#include <sys/defs.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[], char* envp[])
{
    sleep(strtoInt(argv[1]));
    return 0;
}
