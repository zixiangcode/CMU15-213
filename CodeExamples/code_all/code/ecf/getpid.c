#include "../src/csapp.c"

int main() 
{
    pid_t pid = getpid();
    pid_t ppid = getppid();

    printf("pid=%d ppid=%d\n", pid, ppid);

    exit(0);
}
