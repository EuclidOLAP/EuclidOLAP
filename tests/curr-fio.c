#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char *argv[])
{
    printf("argc = %d\n", argc);

    if (argc > 1)
    {
        FILE *fd = fopen("/tmp/curr-fio.txt", "a");

        fprintf(fd, "%s\n", argv[1]);

        fflush(fd);
        fclose(fd);
    }

    fprintf(stdout, ">>> std out <<<\n");
    fprintf(stderr, ">>> std err <<<\n");

    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    for (int i = 0; i < 10; i++)
    {
        printf("%d\n", i + 1);
        usleep(1000000);
    }

    return 0;
}