#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

long counter = 0;

int main(int argc, char *argv[])
{
    for (size_t i = 0; i < 999333111; i++)
    {
        long val = counter;
        long res = __atomic_add_fetch(&counter, 1, val);
        if (!(i % 10000))
            printf("res = %ld --- counter = %ld\n", res, counter);
    }

    printf("!!! >  --- counter = %ld\n", counter);

    return 0;
}