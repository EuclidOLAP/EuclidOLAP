#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h> // for close

#include "cfg.h"
//#include "net.h"
#include "command.h"
#include "utils.h"

// defined in <sys/socket.h>
// extern ssize_t send (int __fd, const void *__buf, size_t __n, int __flags);

// extern void *parse_mdx(char *mdx);

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Please enter the execution statement.\n");
		return EXIT_SUCCESS;
	}

	int i, statement_len = 0;
	for (i = 1; i < argc; i++)
	{
		statement_len += strlen(argv[i]) + 1;
	}
	char *statement = obj_alloc(statement_len, OBJ_TYPE__RAW_BYTES);
	for (i = 1; i < argc; i++)
	{
		strncat(statement, argv[i], strlen(argv[i]));
		if (i < argc - 1)
			strcat(statement, " ");
	}

	// parse_mdx(statement);

	char *args[2];
	args[0] = argv[0];
	args[1] = obj_alloc(16, OBJ_TYPE__RAW_BYTES);
	strcpy(args[1], "--p:mode=client");

	// init_cfg(argc, argv);
	init_cfg(2, args);
	/* TODO bug
	 * If there is an '=' in the MDX script, it will cause a parameter parsing bug,
	 * so currently no command line parameter parsing is performed.
	 */

	EuclidConfig *cfg = get_cfg();

	// The client tool does not need to start the task processing thread.
	cfg->ec_threads_count = 0;

	init_command_module();

	int sock_fd;
	sock_conn_to(&sock_fd, cfg->cli_ctrl_node_host, cfg->cli_ctrl_node_port);

	EuclidCommand *terml_ctrl_ec = get_const_command_intent(INTENT__TERMINAL_CONTROL);

	send(sock_fd, terml_ctrl_ec->bytes, ec_get_capacity(terml_ctrl_ec), 0);

	void *buf = NULL;
	size_t buf_len;
	if (read_sock_pkg(sock_fd, &buf, &buf_len) < 0)
	{
		// if (buf)
		// 	_release_mem_(buf);
		goto _exit_;
	}

	EuclidCommand *allow_f_serv = create_command(buf);
	if (ec_get_intent(allow_f_serv) != INTENT__ALLOW)
	{
		goto _exit_;
	}

	// statement is not a file
	if (access(statement, F_OK) != 0) {
		EuclidCommand *mdx_ec = build_intent_command_mdx(statement);
		send(sock_fd, mdx_ec->bytes, ec_get_capacity(mdx_ec), 0);

		read_sock_pkg(sock_fd, &buf, &buf_len);
		EuclidCommand *result = create_command(buf);
		if (ec_get_intent(result) == INTENT__SUCCESSFUL) {
			printf("\n%s\n", result->bytes + SZOF_USG_INT + SZOF_USG_SHORT);
		} else {
			printf("\nfailure\n");
		}

		goto _exit_;
	}

	// statement is a file name
	FILE *fd = fopen(statement, "r");
	fseek(fd,0,SEEK_END);
	long fsize = ftell(fd);
	rewind(fd);
	char *content = obj_alloc(fsize + 1, OBJ_TYPE__RAW_BYTES);
	fread(content,fsize,1,fd);
	fclose(fd);

	StrArr *scripts = str_split(content, "---");
	obj_release(content);

	for (int i=0; i<scripts->length; i++) {

		char *exe_stat = str_arr_get(scripts, i);
		char *last_char = exe_stat + strlen(exe_stat) - 1;

		while (*exe_stat == '\n')
			exe_stat++;

		while (*last_char == '\n') {
			*last_char = 0;
			--last_char;
		}

		EuclidCommand *mdx_ec = build_intent_command_mdx(exe_stat);

		printf("-----------------------------------------------------------------------------------------------------------\n");
		if (strlen(exe_stat) < 200) {
			printf("%s\n", exe_stat);
		} else {
			exe_stat[199] = 0;
			printf("%s ...\n", exe_stat);
		}

		send(sock_fd, mdx_ec->bytes, ec_get_capacity(mdx_ec), 0);

		read_sock_pkg(sock_fd, &buf, &buf_len);
		EuclidCommand *result = create_command(buf);
		if (ec_get_intent(result) == INTENT__SUCCESSFUL) {
			printf("\n%s\n", result->bytes + SZOF_USG_INT + SZOF_USG_SHORT);
		} else {
			printf("\nfailure\n");
		}

		obj_release(mdx_ec->bytes);
		obj_release(mdx_ec);
		obj_release(result->bytes);
		obj_release(result);
	}

_exit_:
	close(sock_fd);

	return 0;
}
