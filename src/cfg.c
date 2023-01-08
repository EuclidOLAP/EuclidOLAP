#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "log.h"
#include "cfg.h"
#include "utils.h"

static char __program_mode__ = 0;

static EuclidConfig cfg;

static int fetch_param(char *param);

static int set_param(char *p_key, char *p_val);

static int load_conf_files();

void set_program_mode(char mode)
{
	assert(mode == MASTER_MODE || mode == WORKER_MODE || mode == CLIENT_MODE);
	__program_mode__ = mode;
}

int init_cfg(int argc, char *argv[])
{
	memset(&cfg, 0, sizeof(cfg));

	// hard-coded default values
	cfg.mode = __program_mode__ ? __program_mode__ : MASTER_MODE;
	cfg.port = 8760;
	cfg.ec_threads_count = 1;
	cfg.program_path = argv[0];

	// values of configuration file
	load_conf_files();

	/**
	 * Parse the command line arguments, ignoring the 0th
	 * because the 0th is the program execution path.
	 */
	for (int i = 1; i < argc; i++)
	{
		fetch_param(argv[i]);
	}

	// create meta and data folders
	if (access("meta", F_OK) != 0)
		mkdir("meta", S_IRWXU);

	if (access("data", F_OK) != 0)
		mkdir("data", S_IRWXU);

	switch (cfg.mode)
	{
	case MASTER_MODE:
	case WORKER_MODE:
		log_print("info - node mode [ %c ]\n", cfg.mode);
		break;
	case CLIENT_MODE:
		break;
	default:
		log_print("[ error ] Program exits. Caused by the wrong program startup mode.\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int fetch_param(char *param)
{
	char *param_key = param;
	char *param_val = strchr(param, '=');

	if (param_val == NULL)
		return 0;

	*param_val = 0;
	param_val++;
	return set_param(param_key, param_val);
}

int set_param(char *p_key, char *p_val)
{
	if (strcmp(p_key, "--p:mode") == 0)
	{
		if (strcmp(p_val, "master") == 0)
		{
			cfg.mode = MASTER_MODE;
		}
		else if (strcmp(p_val, "worker") == 0)
		{
			cfg.mode = WORKER_MODE;
		}
		else if (strcmp(p_val, "client") == 0)
		{
			cfg.mode = CLIENT_MODE;
		}
		else
		{
			log_print("Unknown mode %s.\n", p_val);
			exit(1);
		}
	}
	else if (strcmp(p_key, "cli.ctrl.node.host") == 0)
	{
		strcpy(cfg.cli_ctrl_node_host, p_val);
	}
	else if (strcmp(p_key, "cli.ctrl.node.port") == 0)
	{
		cfg.cli_ctrl_node_port = atoi(p_val);
	}
	else if (strcmp(p_key, "--host") == 0)
	{
		cfg.host = p_val;
	}
	else if (strcmp(p_key, "--file") == 0)
	{
		cfg.file = p_val;
	}
	else if (strcmp(p_key, "--p:port") == 0 || strcmp(p_key, "--port") == 0)
	{
		cfg.port = atoi(p_val);
		if (cfg.port < 0 || cfg.port > 65535)
		{
			log_print("Invalid port %d, valid port 0 ~ 65535\n", cfg.port);
			exit(EXIT_FAILURE);
		}
	}
	else if (strcmp(p_key, "--p:command-threads-count") == 0)
	{
		cfg.ec_threads_count = atoi(p_val);
	}
	else if (strcmp(p_key, "--p:join") == 0)
	{
		StrArr *arr = str_split(p_val, ":");
		strcpy(cfg.parent_node_ip, str_arr_get(arr, 0));
		cfg.parent_node_port = atoi(str_arr_get(arr, 1));
		destory_StrArr(arr);
	}
	else
	{
		log_print("Unknown parameter type %s=%s.\n", p_key, p_val);
		exit(EXIT_FAILURE);
	}

	return 0;
}

EuclidConfig *get_cfg()
{
	return &cfg;
}

static int load_conf_files()
{
	char *cfg_file_name;
	if (cfg.mode == CLIENT_MODE)
	{
		cfg_file_name = DEF_CLI_CONF;
	}
	else
	{
		// No configuration files are currently required when running in server mode.
		return 0;
	}

	int len = strlen(cfg.program_path) + 32;
	char conf_path[len];
	memset(conf_path, 0, len);
	strcpy(conf_path, cfg.program_path);

	int i;
	for (i = len - 1; i >= 0; i--)
	{
		if (conf_path[i] != '/')
			conf_path[i] = '\0';
		else
			break;
	}

	strcat(conf_path, cfg_file_name);

	if (access(conf_path, F_OK) != 0) {
		// the config file was not existed.

		if (cfg.mode == CLIENT_MODE) {
			// set the default ip and port of euclid-olap server, when it's client mode.
			cfg.cli_ctrl_node_port = 8760;
			strcpy(cfg.cli_ctrl_node_host, "127.0.0.1");
		}

		return 0;
	}

	FILE *conf_fp = fopen(conf_path, "r");

	int buf_len = 256;
	char buf_arr[buf_len];

	while (fgets(buf_arr, buf_len, conf_fp) != NULL)
	{
		char *last_char = buf_arr + strlen(buf_arr) - 1;
		while (*last_char == '\n')
		{
			*last_char = 0;
			--last_char;
		}

		fetch_param(buf_arr);
		memset(buf_arr, 0, buf_len);
	}

	fclose(conf_fp);

	return 0;
}
