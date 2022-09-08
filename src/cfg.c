#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "utils.h"

static EuclidConfig cfg;

static int fetch_param(char *param);

static int set_param(char *p_key, char *p_val);

static int load_conf_files();

int init_cfg(int argc, char *argv[])
{
	memset(&cfg, 0, sizeof(cfg));

	// hard-coded default values
	cfg.mode = MASTER_MODE;
	cfg.port = 8760;
	cfg.ec_threads_count = 1;
	cfg.program_path = argv[0];

	// values of configuration file
	load_conf_files();

	// program parameter values
	int i;
	for (i = 1; i < argc; i++)
	{
		fetch_param(argv[i]);
	}

	printf("info - node mode [ %c ]\n", cfg.mode);

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
		else
		{
			printf("Unknown mode %s.\n", p_val);
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
	else if (strcmp(p_key, "--p:port") == 0)
	{
		cfg.port = atoi(p_val);
		if (cfg.port < 0 || cfg.port > 65535)
		{
			printf("Invalid port %d, valid port 0 ~ 65535\n", cfg.port);
			exit(1);
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
		printf("Unknown parameter type %s=%s.\n", p_key, p_val);
		exit(1);
	}

	return 0;
}

EuclidConfig *get_cfg()
{
	return &cfg;
}

static int load_conf_files()
{
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

	strcat(conf_path, DEF_CLI_CONF);

	FILE *cli_conf_fp = fopen(conf_path, "r");

	int buf_len = 256;
	char buf_arr[buf_len];

	while (fgets(buf_arr, buf_len, cli_conf_fp) != NULL)
	{
		fetch_param(buf_arr);
		memset(buf_arr, 0, buf_len);
	}

	fclose(cli_conf_fp);

	return 0;
}
