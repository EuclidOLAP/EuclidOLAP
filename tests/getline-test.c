#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <termios.h>

int main(int argc, char *argv[])
{

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, stdin)) != -1)
	{
		printf("Retrieved line of length %zu, %u :\n", read, len);
		printf("%s", line);
	}

	free(line);

	return 0;
}