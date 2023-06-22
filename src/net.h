#ifndef EUCLID__NET_H
#define EUCLID__NET_H 1

#include <arpa/inet.h>

#include "command.h"
#include "utils.h"

void net_init();

typedef struct sock_intent_thread
{
	pthread_t thread_id;

	// When the SockIntentThread object represents a Worker-Node that joins the cluster,
	// this field stores the ID of the Worker-Node.
	unsigned long worker_id;

	int sock_fd;
	intent inte;
} SockIntentThread;

int net_service_startup();

// int sit_startup(SockIntentThread *sit);

int join_cluster();

int sit_send_command(SockIntentThread *sit, EuclidCommand *ec);

// Randomly obtain a socket corresponding to a downstream node.
int random_child_sock();

// The socket of the worker node is obtained by worker_id.
int work_node_sock(unsigned long worker_id);

SockIntentThread *master_sit();

// get the count of children-nodes in same cluster
__uint32_t d_nodes_count();


ArrayList *worker_nodes();


/*************************************************************************************
 * SockIntentThread(struct sock_intent_thread) functions                             *
 *************************************************************************************/

// SockIntentThread *create_SockIntentThread(int sock_fd);
SockIntentThread *sit_alloc(int sock_fd);

void sit_release(SockIntentThread *sit);

#endif
