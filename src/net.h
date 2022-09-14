#ifndef EUCLID__NET_H
#define EUCLID__NET_H 1

#include <arpa/inet.h>

#include "command.h"

void net_init();

typedef struct sock_intent_thread
{
	int sock_fd;
	pthread_t thread_id;
} SockIntentThread;

int net_service_startup();

SockIntentThread *create_SockIntentThread(int sock_fd);

// int sit_startup(SockIntentThread *sit);

int join_cluster();

int sit_send_command(SockIntentThread *sit, EuclidCommand *ec);

// Randomly obtain a socket corresponding to a downstream node.
int random_child_sock();

// get the count of children-nodes in same cluster
__uint32_t d_nodes_count();

#endif
