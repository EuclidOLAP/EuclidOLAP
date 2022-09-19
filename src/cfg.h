#ifndef EUCLID__CFG_H
#define EUCLID__CFG_H 1

// #define PARAM_KEY__MODE "--p:mode"
// #define PARAM_KEY__PORT "--p:port"
// #define PARAM_KEY__JOIN "--p:join"

#define MASTER_MODE 'm'
#define WORKER_MODE 'w'
#define CLIENT_MODE 'c'

#define DEF_CONF "euclid.conf"
#define DEF_CLI_CONF "euclid-cli.conf"

typedef struct euclid_cfg
{
	char parent_node_ip[16];
	int parent_node_port;

	int port;

	int ec_threads_count;

	char *program_path;

	char cli_ctrl_node_host[16];
	int cli_ctrl_node_port;

	char mode;
} EuclidConfig;

int init_cfg(int argc, char *argv[]);

EuclidConfig *get_cfg();

void set_program_mode(char mode);

#endif
