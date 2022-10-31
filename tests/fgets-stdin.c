#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <termios.h>

int setlflag(int flags, int enable)
{
	struct termios attr;
	tcgetattr(STDIN_FILENO, &attr);
	if (enable)
	{
		attr.c_lflag |= flags;
	}
	else
	{
		attr.c_lflag &= ~flags;
	}
	return tcsetattr(STDIN_FILENO, TCIFLUSH, &attr);
}

int main()
{
	size_t len = 0x01UL << 8;
	char str[len];

	while (1)
	{
		setlflag(ECHO, 0);
		if (fgets(str, len, stdin) != NULL)
		{
			puts(str);
			if (strcmp(str, "exit\n") == 0)
			{
				break;
			}
		}
	}

	return 0;
}