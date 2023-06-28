#ifndef EUCLID__CFG_H
#define EUCLID__CFG_H 1


#define PROFILES_MAX_LEN     300

#define PROFILE_DIMENSIONS   "/profiles/dimensions"
#define PROFILE_MEMBERS      "/profiles/members"
#define PROFILE_LEVELS       "/profiles/levels"
#define PROFILE_CUBES        "/profiles/cubes"
#define PROFILE_HIERARCHIES  "/profiles/hierarchies"
#define PROFILE_CUBE_PREFIX  "/profiles/cube_"


// #define PARAM_KEY__MODE "--p:mode"
// #define PARAM_KEY__PORT "--p:port"
// #define PARAM_KEY__JOIN "--p:join"


// stand-alone
#define MODE_STAND_ALONE 's'
// master
#define MODE_MASTER 'm'
// worker
#define MODE_WORKER 'w'
// client
#define MODE_CLIENT 'c'


#define OLAP_SERVER_CONF "/config/euclid-svr.conf"
// #define DEF_CONF "euclid-svr.conf"
// #define DEF_CLI_CONF "euclid-cli.conf"


#define CFG_WORKERID_LEN_LIMIT 32

typedef struct euclid_cfg
{
	// char *program_path;

	char *host; // the IP or domain of server, be used when client mode
	char *file; // the file be executed, be used when client mode

	char parent_node_ip[16];
	char cli_ctrl_node_host[16];

	// the worker id of service running on worker mode in cluster.
	char worker_id[CFG_WORKERID_LEN_LIMIT];

	int parent_node_port;
	int port;
	int ec_threads_count;
	int cli_ctrl_node_port;

	char mode;

	struct
	{
		char dimensions[PROFILES_MAX_LEN + 1];
		char members[PROFILES_MAX_LEN + 1];
		char levels[PROFILES_MAX_LEN + 1];
		char hierarchies[PROFILES_MAX_LEN + 1];
		char cubes[PROFILES_MAX_LEN + 1];
		char cube_prefix[PROFILES_MAX_LEN + 1];
	} profiles;

} EuclidConfig;

int init_cfg(int argc, char *argv[]);

EuclidConfig *get_cfg();

void set_program_mode(char mode);

#endif
