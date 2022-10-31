#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <termios.h>

int main(int argc, char *argv[])
{
	char command[128];

	while (1)
	{
		memset(command, 0, 128);
		printf("cli > ");
		if (fgets(command, 128, stdin) != NULL)
		{
			if (strcmp(command, "exit\n") == 0)
				break;
			if (strcmp(command, "\n") == 0)
				continue;
			printf("do something ... %s", command);
		}
	}

	return 0;
}