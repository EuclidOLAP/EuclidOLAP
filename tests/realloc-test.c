#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[])
{

	void *p1 = malloc(1000);
	void *pA = malloc(8);

	printf("p 1 = %p\n", p1);
	printf("p A = %p\n", pA);

	printf("pA - p1 = %lu\n", (unsigned long)pA - (unsigned long)p1);

	void *p2 = realloc(p1, 2000);
	printf("p 1 = %p\n", p1);
	printf("p 2 = %p\n", p2);

	// free(p1);
	free(pA);
	free(p2);

	return 0;
}