#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h> // for close
#include <sys/wait.h>

#include "cfg.h"
#include "command.h"
#include "utils.h"
#include "printer.h"

#define __PROMPT_TEXT__ ">>>>>> Please enter a command, exit enter the 'q' >>>>>>"
#define __RESULT_TEXT__ ">>>>>>>>>>>>>>>>>>>>>>>> RESULT >>>>>>>>>>>>>>>>>>>>>>>>"

static void __prompt__(char *before, char *after)
{
	printf("%s%s%s", before ? before : "", __PROMPT_TEXT__, after ? after : "");
}

static void ctrl_c_act(int sig_no)
{
	// printf("\n");
	__prompt__("\n", "\n\n");
}

/**
 * euclid [--host=192.168.66.235] [--port=8760] [--file=demo-meta.txt]
 */
int main(int argc, char *argv[])
{
	set_program_mode(MODE_CLIENT);

	init_cfg(argc, argv);

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
		goto _exit_;
	}

	EuclidCommand *allow_f_serv = create_command(buf);
	if (ec_get_intent(allow_f_serv) != INTENT__ALLOW)
	{
		goto _exit_;
	}

	// statement is not a file
	if (cfg->file == NULL)
	{

		signal(SIGINT, ctrl_c_act);

		ArrayList *input_ls = als_new(32, "char *", DIRECT, NULL);

		size_t line_len = 1024 * 2;
		char *input_line = obj_alloc(line_len, OBJ_TYPE__RAW_BYTES);
		MemAllocMng *mam = MemAllocMng_new();
		__prompt__(NULL, "\n\n");
		while (1)
		{
			// printf("olapcli > ");
			fgets(input_line, line_len, stdin);

			if (strlen(input_line) >= line_len - 1)
			{
				printf("There are too many characters entered in one line, restart the client and use line breaks.\n");
				goto _exit_;
			}

			if (strcmp(input_line, "q\n") == 0 || strcmp(input_line, "exit\n") == 0 || strcmp(input_line, "quit\n") == 0)
				goto _exit_;

			char *replica_line = mam_hlloc(mam, strlen(input_line) + 1);
			strcpy(replica_line, input_line);

			als_add(input_ls, replica_line);

			if (strlen(replica_line) > 1 && replica_line[strlen(replica_line) - 2] == ';')
			{
				int statement_len = 1; // als_size(input_ls);

				for (int i = 0; i < als_size(input_ls); i++)
				{
					statement_len += strlen(als_get(input_ls, i));
				}

				char *statement = mam_hlloc(mam, statement_len);

				for (int i = 0; i < als_size(input_ls); i++)
				{
					strcat(statement, als_get(input_ls, i));
				}

				EuclidCommand *mdx_ec = build_intent_command_mdx(statement);
				// ec_change_intent(mdx_ec, INTENT__MDX_EXPECT_RESULT_TXT);

				send(sock_fd, mdx_ec->bytes, ec_get_capacity(mdx_ec), 0);

				read_sock_pkg(sock_fd, &buf, &buf_len);
				EuclidCommand *result = create_command(buf);

				printf("\n%s\n", __RESULT_TEXT__);

				switch (ec_get_intent(result))
				{
				case INTENT__MULTIDIM_RESULT_BIN:
					print_mdrs(result);
					break;
				case INTENT__SUCCESSFUL:
				case INTENT__EXE_RESULT_DESC:
				case INTENT__FAILURE:
					printf("\n%s\n", result->bytes + SZOF_USG_INT + SZOF_USG_SHORT);
					break;
				default:
					printf("\nUnknown Information!\n");
				}

				obj_release(mdx_ec->bytes);
				obj_release(mdx_ec);
				obj_release(result->bytes);
				obj_release(result);

				for (int i = als_size(input_ls) - 1; i >= 0; i--)
				{
					als_rm_index(input_ls, i);
				}

				mam_reset(mam);
				__prompt__(NULL, "\n\n");
			}
		}

		goto _exit_;
	}

	// statement is a file name
	FILE *fd = fopen(cfg->file, "r");
	fseek(fd, 0, SEEK_END);
	long fsize = ftell(fd);
	rewind(fd);
	char *content = obj_alloc(fsize + 1, OBJ_TYPE__RAW_BYTES);
	fread(content, fsize, 1, fd);
	fclose(fd);

	StrArr *scripts = str_split(content, "---");
	obj_release(content);

	for (int i = 0; i < scripts->length; i++)
	{

		char *exe_stat = str_arr_get(scripts, i);
		char *last_char = exe_stat + strlen(exe_stat) - 1;

		while (*exe_stat == '\n')
			exe_stat++;

		while (*last_char == '\n')
		{
			*last_char = 0;
			--last_char;
		}

		EuclidCommand *mdx_ec = build_intent_command_mdx(exe_stat);
		// ec_change_intent(mdx_ec, INTENT__MDX_EXPECT_RESULT_TXT);

		printf("-----------------------------------------------------------------------------------------------------------\n");
		if (strlen(exe_stat) < 200)
		{
			printf("%s\n", exe_stat);
		}
		else
		{
			exe_stat[199] = 0;
			printf("%s ...\n", exe_stat);
		}

		send(sock_fd, mdx_ec->bytes, ec_get_capacity(mdx_ec), 0);

		read_sock_pkg(sock_fd, &buf, &buf_len);
		EuclidCommand *result = create_command(buf);

		switch (ec_get_intent(result))
		{
		case INTENT__MULTIDIM_RESULT_BIN:
			print_mdrs(result);
			break;
		case INTENT__SUCCESSFUL:
		case INTENT__EXE_RESULT_DESC:
		case INTENT__FAILURE:
			printf("\n%s\n", result->bytes + SZOF_USG_INT + SZOF_USG_SHORT);
			break;
		default:
			printf("\nUnknown Information!\n");
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
