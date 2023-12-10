/* $begin forkprob1 */
#include "../src/csapp.c"

int main() 
{
    int i;

    for (i = 0; i < 2; i++) 
	Fork();
    printf("hello\n");
    exit(0);
}
/* $end forkprob1 */
