#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>

int main(int argc, char *argv[])
{
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