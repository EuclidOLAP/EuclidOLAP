#include <stdlib.h>
// #include <pthread.h>

#include "log.h"
#include "cfg.h"
#include "net.h"
#include "command.h"
#include "vce.h"

// extern pthread_mutex_t mutex_AAA;

int main(int argc, char *argv[])
{
	// pthread_mutex_init(&mutex_AAA, NULL);

	log__set_log_file("log/euclid.log");

	init_cfg(argc, argv);

	mdd_init();

	mdd_load();

	vce_init();

	vce_load();

	init_command_module();

	net_init();

	if (get_cfg()->mode == WORKER_MODE)
		join_cluster();

	net_service_startup();

	return EXIT_SUCCESS;
}
