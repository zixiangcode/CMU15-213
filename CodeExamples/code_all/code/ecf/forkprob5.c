/* $begin forkprob5 */
#include "../src/csapp.c"

void doit() 
{
    if (Fork() == 0) {
	Fork();
	printf("hello\n");
	exit(0);
    }
    return;
}

int main() 
{
    doit();
    printf("hello\n");
    exit(0);
}
/* $end forkprob5 */
