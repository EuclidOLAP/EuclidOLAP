#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>

// #include <stdio.h>
// #include <pthread.h>
// #include <unistd.h>
#include <sys/wait.h>
// #include <sys/types.h>

void Stop(int signo)
{
	printf("oops! stop!!!\n");
	// _exit(0);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, Stop);

	while (1)
	{
		char *input = readline("olapcli > ");

		if (*input == 0)
			continue;

		if (strcmp(input, "exit") == 0)
			break;

		printf("do something ... %s\n", input);

		free(input);
	}

	return 0;
}