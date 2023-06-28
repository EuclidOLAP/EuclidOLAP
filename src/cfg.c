#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "env.h"
#include "log.h"
#include "cfg.h"
#include "utils.h"

extern OLAPEnv olap_env;

static char __program_mode__ = 0;

static EuclidConfig cfg;

static int fetch_param(char *param);

static int set_param(char *p_key, char *p_val);

static int load_conf_files();

void set_program_mode(char mode)
{
	assert(mode == MODE_STAND_ALONE || mode == MODE_MASTER || mode == MODE_WORKER || mode == MODE_CLIENT);
	__program_mode__ = mode;
}

int init_cfg(int argc, char *argv[])
{
	memset(&cfg, 0, sizeof(cfg));

	// hard-coded default values
	cfg.mode = __program_mode__ ? __program_mode__ : MODE_STAND_ALONE;
	cfg.port = 8760;
	cfg.ec_threads_count = 1;
	// cfg.program_path = argv[0];

	if (cfg.mode != MODE_CLIENT) {
		if (strlen(olap_env.OLAP_HOME) + strlen(PROFILE_DIMENSIONS) > PROFILES_MAX_LEN) {
			fprintf(stderr, "[exit cause by error] '%s%s' length over %d.\n", olap_env.OLAP_HOME, PROFILE_DIMENSIONS, PROFILES_MAX_LEN);
			exit(EXIT_FAILURE);
		} else {
			sprintf(cfg.profiles.dimensions, "%s%s", olap_env.OLAP_HOME, PROFILE_DIMENSIONS);
		}
		
		if (strlen(olap_env.OLAP_HOME) + strlen(PROFILE_HIERARCHIES) > PROFILES_MAX_LEN) {
			fprintf(stderr, "[exit cause by error] '%s%s' length over %d.\n", olap_env.OLAP_HOME, PROFILE_HIERARCHIES, PROFILES_MAX_LEN);
			exit(EXIT_FAILURE);
		} else {
			sprintf(cfg.profiles.hierarchies, "%s%s", olap_env.OLAP_HOME, PROFILE_HIERARCHIES);
		}
		
		if (strlen(olap_env.OLAP_HOME) + strlen(PROFILE_LEVELS) > PROFILES_MAX_LEN) {
			fprintf(stderr, "[exit cause by error] '%s%s' length over %d.\n", olap_env.OLAP_HOME, PROFILE_LEVELS, PROFILES_MAX_LEN);
			exit(EXIT_FAILURE);
		} else {
			sprintf(cfg.profiles.levels, "%s%s", olap_env.OLAP_HOME, PROFILE_LEVELS);
		}
		
		if (strlen(olap_env.OLAP_HOME) + strlen(PROFILE_MEMBERS) > PROFILES_MAX_LEN) {
			fprintf(stderr, "[exit cause by error] '%s%s' length over %d.\n", olap_env.OLAP_HOME, PROFILE_MEMBERS, PROFILES_MAX_LEN);
			exit(EXIT_FAILURE);
		} else {
			sprintf(cfg.profiles.members, "%s%s", olap_env.OLAP_HOME, PROFILE_MEMBERS);
		}
		
		if (strlen(olap_env.OLAP_HOME) + strlen(PROFILE_CUBES) > PROFILES_MAX_LEN) {
			fprintf(stderr, "[exit cause by error] '%s%s' length over %d.\n", olap_env.OLAP_HOME, PROFILE_CUBES, PROFILES_MAX_LEN);
			exit(EXIT_FAILURE);
		} else {
			sprintf(cfg.profiles.cubes, "%s%s", olap_env.OLAP_HOME, PROFILE_CUBES);
		}
		
		if (strlen(olap_env.OLAP_HOME) + strlen(PROFILE_CUBE_PREFIX) > PROFILES_MAX_LEN) {
			fprintf(stderr, "[exit cause by error] '%s%s' length over %d.\n", olap_env.OLAP_HOME, PROFILE_CUBE_PREFIX, PROFILES_MAX_LEN);
			exit(EXIT_FAILURE);
		} else {
			sprintf(cfg.profiles.cube_prefix, "%s%s", olap_env.OLAP_HOME, PROFILE_CUBE_PREFIX);
		}
	}

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

	// // create meta and data folders
	// if (access("meta", F_OK) != 0)
	// 	mkdir("meta", S_IRWXU);

	// if (access("data", F_OK) != 0)
	// 	mkdir("data", S_IRWXU);

	switch (cfg.mode)
	{
	case MODE_STAND_ALONE:
	case MODE_MASTER:
	case MODE_WORKER:
		log_print("info - node mode [ %c ]\n", cfg.mode);
		break;
	case MODE_CLIENT:
		break;
	default:
		log_print("[ error ] Program exits. Caused by the wrong program startup mode.\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int fetch_param(char *param)
{
	if (*param == '#')
		return 1;

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
	if (strcmp(p_key, "--p:mode") == 0 || strcmp(p_key, "mode") == 0)
	{
		if (strcmp(p_val, "stand-alone") == 0)
		{
			cfg.mode = MODE_STAND_ALONE;
		}
		else if (strcmp(p_val, "master") == 0)
		{
			cfg.mode = MODE_MASTER;
		}
		else if (strcmp(p_val, "worker") == 0)
		{
			cfg.mode = MODE_WORKER;
		}
		else if (strcmp(p_val, "client") == 0)
		{
			cfg.mode = MODE_CLIENT;
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
	else if (strcmp(p_key, "--p:port") == 0 || strcmp(p_key, "--port") == 0 || strcmp(p_key, "port") == 0)
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
	else if (strcmp(p_key, "--p:join") == 0 || strcmp(p_key, "master") == 0)
	{
		StrArr *arr = str_split(p_val, ":");
		strcpy(cfg.parent_node_ip, str_arr_get(arr, 0));
		cfg.parent_node_port = atoi(str_arr_get(arr, 1));
		destory_StrArr(arr);
	}
	else if (strcmp(p_key, "worker.id") == 0)
	{
		assert(strlen(p_val) < CFG_WORKERID_LEN_LIMIT);
		strcpy(cfg.worker_id, p_val);
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
	// char *cfg_file_name;
	// if (cfg.mode == MODE_CLIENT)
	// {
	// 	cfg_file_name = DEF_CLI_CONF;
	// }
	// else
	// {
	// 	cfg_file_name = DEF_CONF;
	// }

	// int len = strlen(cfg.program_path) + 32;
	// char conf_path[len];
	// memset(conf_path, 0, len);
	// strcpy(conf_path, cfg.program_path);

	// int i;
	// for (i = len - 1; i >= 0; i--)
	// {
	// 	if (conf_path[i] != '/')
	// 		conf_path[i] = '\0';
	// 	else
	// 		break;
	// }

	// char cfg_file_name[512];
	// sprintf(cfg_file_name, "%s%s", olap_env.OLAP_HOME, OLAP_SERVER_CONF);

	// strcat(conf_path, cfg_file_name);

	// if (access(conf_path, F_OK) != 0)
	// {
	// 	// the config file was not existed.

	// 	if (cfg.mode == MODE_CLIENT)
	// 	{
	// 		// set the default ip and port of euclid-olap server, when it's client mode.
	// 		cfg.cli_ctrl_node_port = 8760;
	// 		strcpy(cfg.cli_ctrl_node_host, "127.0.0.1");
	// 	}

	// 	return 0;
	// }

	if (cfg.mode == MODE_CLIENT)
	{
		// set the default ip and port of euclid-olap server, when it's client mode.
		cfg.cli_ctrl_node_port = 8760;
		strcpy(cfg.cli_ctrl_node_host, "127.0.0.1");
		return 0;
	}

	char server_conf[512];
	assert(strlen(olap_env.OLAP_HOME) + strlen(OLAP_SERVER_CONF) < 511);
	memset(server_conf, 0, 512);
	sprintf(server_conf, "%s%s", olap_env.OLAP_HOME, OLAP_SERVER_CONF);

	FILE *conf_fp = fopen(server_conf, "r");

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
