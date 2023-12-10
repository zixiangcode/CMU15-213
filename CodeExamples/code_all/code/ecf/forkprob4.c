/* $begin forkprob4 */
#include "../src/csapp.c"

void doit() 
{
    Fork();
    Fork();
    printf("hello\n");
    return;
}

int main() 
{
    doit();
    printf("hello\n");
    exit(0);
}
/* $end forkprob4 */


