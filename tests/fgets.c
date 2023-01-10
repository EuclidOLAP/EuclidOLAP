#include <stdio.h>
// #include <stdlib.h>

#define LEN 6
int main(int argc, char *argv[])
{
    char src[LEN];
    while (1)
    {
        printf("@@@please enter:\n");
        fgets(src, LEN, stdin);
        printf("@@@your enter is:\n");
        fputs(src, stdout);
    }
    return 0;
}