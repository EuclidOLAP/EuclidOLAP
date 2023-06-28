#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // for pthread_create
#include <string.h>	 // for memset
#include <unistd.h>	 // for close
#include <semaphore.h>

#include "log.h"
#include "net.h"
#include "mdd.h"
#include "cfg.h"
#include "utils.h"
#include "command.h"

// static MemAllocMng *net_mam;

// static ArrayList *downstream_sockets;

static ArrayList *worker_sits;

static SockIntentThread *_master__sit_;

static void *sit_startup(void *sit_addr);

static int sit_close(SockIntentThread *sit);

// static int send_sock_inte(SockIntentThread *sit, EuclidCommand *ec);

static void *command_receiving_thread(void *addr);

static void receive_command_loop(SockIntentThread *sit);

void net_init()
{
	// net_mam = MemAllocMng_new();
	// log_print("@@MAM@@ -->> net_mam = %p\n", net_mam);
}

int net_service_startup()
{
	EuclidConfig *cfg = get_cfg();

	int ss_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Set the socket option to enable address reuse functionality.
	int optval = 1;
	setsockopt(ss_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	struct sockaddr_in ss_addr;
	memset(&ss_addr, 0, sizeof(ss_addr));

	ss_addr.sin_family = AF_INET;
	ss_addr.sin_port = htons(cfg->port);
	ss_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(ss_fd, (struct sockaddr *)&ss_addr, sizeof(ss_addr)) != 0)
	{
		log_print("program exit. cause: bind failed.");
		exit(1);
	}

	if (listen(ss_fd, 5) != 0)
	{
		log_print("program exit. cause: listen failed.");
		exit(1);
	}
	log_print("Net service startup on port %d\n", cfg->port);

	socklen_t cs_len = sizeof(struct sockaddr_in);

	// downstream_sockets = als_new(16, "downstream_sockets, int *", SPEC_MAM, net_mam);
	worker_sits = als_new(16, "SockIntentThread *", DIRECT, NULL);
	als_sync(worker_sits);

	while (1)
	{
		struct sockaddr_in sock;

		int sock_fd = accept(ss_fd, (struct sockaddr *)&sock, &cs_len);
		log_print("info - accept client. sock_fd = %d\n", sock_fd);
		if (sock_fd < 3)
		{ // 0 stdin, 1 stdout, 2 stderr, -1 INVALID_SOCKET
			log_print("warning: accept return [%d].\n", sock_fd);
			continue;
		}

		// int *skt_v_p = mam_alloc(sizeof(int), OBJ_TYPE__RAW_BYTES, net_mam, 0); // bug: Memory leak risk.
		// *skt_v_p = sock_fd;
		// als_add(downstream_sockets, skt_v_p);

		// SockIntentThread *sit = create_SockIntentThread(sock_fd);
		SockIntentThread *sit = sit_alloc(sock_fd);

		pthread_create(&(sit->thread_id), NULL, sit_startup, sit);
		pthread_detach(sit->thread_id);
	}

	close(ss_fd);
	return 0;
}

// SockIntentThread *create_SockIntentThread(int sock_fd)
// {
// 	SockIntentThread *sit_p = (SockIntentThread *)mam_alloc(sizeof(SockIntentThread), OBJ_TYPE__SockIntentThread, net_mam, 0);
// 	sit_p->sock_fd = sock_fd;
// 	return sit_p;
// }

SockIntentThread *sit_alloc(int sock_fd) {
	SockIntentThread *sit = obj_alloc(sizeof(SockIntentThread), OBJ_TYPE__SockIntentThread);
	sit->sock_fd = sock_fd;
	sit->inte = INTENT__UNKNOWN;
	return sit;
}

void sit_release(SockIntentThread *sit) {
	obj_release(sit);
}

static void *sit_startup(void *sit_addr)
{
	SockIntentThread *sit = (SockIntentThread *)sit_addr;

	// read data package from socket.
	void *buf = NULL;
	size_t buf_len;
	if (read_sock_pkg(sit->sock_fd, &buf, &buf_len) < 0)
	{
		if (buf)
			obj_release(buf);
		goto _exit_;
	}

	// convert package to command.
	EuclidCommand *command = create_command(buf);

	// check first command.
	intent inte = ec_get_intent(command);

	if (inte == INTENT__TERMINAL_CONTROL) {
		log_print("[ info ] A terminal was connected < sock_fd : %d >\n", sit->sock_fd);
		sit->inte = INTENT__TERMINAL_CONTROL;
	} else if (inte == INTENT__WORKER_JOINS_CLUSTER) {
		sit->inte = INTENT__WORKER_JOINS_CLUSTER;
		sit->worker_id = *(unsigned long *)(buf + sizeof(int) + sizeof(short));
		als_add_sync(worker_sits, sit);
		log_print("[ info ] A worker <ID:%lu> service was connected < sock_fd : %d >\n", sit->worker_id, sit->sock_fd);
	} else {
		// goto _exit_;

		// fail fast
		log_print("[ error] <Fail fast!!!> Process was killed cause by unknown command intent.\n");
		exit(EXIT_FAILURE);
	}

	// ec_release(command);
	obj_release(command->bytes);
	obj_release(command);

	// send allow intention to terminal or child.
	sit_send_command(sit, get_const_command_intent(INTENT__ALLOW));
	receive_command_loop(sit);

_exit_:

	if (sit->inte == INTENT__WORKER_JOINS_CLUSTER)
		als_remove_sync(worker_sits, sit);

	sit_close(sit); // close sit -> sock_fd, and release sit memory.
	sit_release(sit);

	return NULL;
}

static int sit_close(SockIntentThread *sit)
{
	close(sit->sock_fd);
	// _release_mem_(sit);
	return 0;
}

// static int send_sock_inte(SockIntentThread *sit, EuclidCommand *ec)
// {
// 	return send(sit -> sock_fd, ec -> bytes, ec_get_capacity(ec), 0);
// }

int join_cluster()
{
	int to_parent_sock_fd = 0;
	char *p_ip = get_cfg()->parent_node_ip;
	int p_port = get_cfg()->parent_node_port;
	if (sock_conn_to(&to_parent_sock_fd, p_ip, p_port) != 0)
	{
		log_print("[net] error. connect %s:%d\n", p_ip, p_port);
		return -1;
	}

	_master__sit_ = sit_alloc(to_parent_sock_fd);
	sit_send_command(_master__sit_, get_const_command_intent(INTENT__WORKER_JOINS_CLUSTER));

	void *buf = NULL;
	size_t buf_len;
	if (read_sock_pkg(to_parent_sock_fd, &buf, &buf_len) < 0)
	{
		// fail fast
		log_print("<Fail fast!!!> Lost connection to master server.\n");
		exit(EXIT_FAILURE);
	}

	EuclidCommand *allow_f_serv = create_command(buf);
	if (ec_get_intent(allow_f_serv) != INTENT__ALLOW)
	{
		// fail fast
		log_print("<Fail fast!!!> Master rejects the current worker from joining the cluster.\n");
		exit(EXIT_FAILURE);
	}

	obj_release(allow_f_serv->bytes);
	obj_release(allow_f_serv);

	pthread_create(&(_master__sit_->thread_id), NULL, command_receiving_thread, _master__sit_);
	pthread_detach(_master__sit_->thread_id);

	return 0;
}

int sit_send_command(SockIntentThread *sit, EuclidCommand *ec)
{
	return send(sit->sock_fd, ec->bytes, ec_get_capacity(ec), 0);
}

static void *command_receiving_thread(void *addr)
{
	SockIntentThread *worker_up_sit = (SockIntentThread *)addr;

	receive_command_loop(worker_up_sit);

	sit_close(worker_up_sit);

	return NULL;
}

static void receive_command_loop(SockIntentThread *sit)
{
	ssize_t min_pkg_size = SZOF_USG_INT + SZOF_USG_SHORT;

	void *buf;
	size_t buf_len;

	while (1)
	{
		// receive command.
		buf = NULL;
		buf_len = 0;
		ssize_t pkg_size = read_sock_pkg(sit->sock_fd, &buf, &buf_len);
		// log_print("INFO - receive_command_loop() ... <<<<<< sit->sock_fd = %d >>>>>> read_sock_pkg, pkg_size = %ld\n", sit->sock_fd, pkg_size);
		if (pkg_size >= min_pkg_size)
		{
			EuclidCommand *task = create_command(buf);

			if (ec_get_intent(task) == INTENT__AGGREGATE_TASK_RESULT) {
				put_agg_task_result(task);
				continue;
			}


			sem_init(&(task->sem), 0, 0);

			/**
			 * Command module process command intention.
			 * Submit the task object to another thread for processing, that thread will increment
			 * the value of the semaphore by 1 after execution is complete.
			 */
			submit_command(task);

			sem_wait(&(task->sem));
			sem_destroy(&(task->sem));

			EuclidConfig *cfg = get_cfg();
			if (cfg->mode != MODE_WORKER) {
				sit_send_command(sit, task->result ? task->result : get_const_command_intent(INTENT__SUCCESSFUL));
			} else if (task->result) {
				sit_send_command(sit, task->result);	
			}

			if (task->result)
			{
				obj_release(task->result->bytes);
				obj_release(task->result);
			}
			obj_release(task->bytes);
			obj_release(task);

			continue;
		}
		if (buf)
			obj_release(buf);

		log_print("warning! receive intention: buf_len = %d\n", buf_len);
		// als_remove(downstream_sockets, &(sit->sock_fd));
		int _c_ = close(sit->sock_fd);
		log_print("-------------------------------------------------------  _c_ = %d\n", _c_);
		break;
	}
}

int random_child_sock()
{
	// __uint32_t r_idx = rand() % als_size(downstream_sockets);
	// return *((int *)als_get(downstream_sockets, r_idx));
	
	__uint32_t r_idx = rand() % als_size(worker_sits);
	SockIntentThread *sit = als_get(worker_sits, r_idx);
	return sit->sock_fd;

	// log_print("[ error ] Process end: caused by an incorrect function call.\n");
	// exit(EXIT_FAILURE);
	// return 0;
}

int work_node_sock(unsigned long worker_id) {
	int sz = als_size(worker_sits);
	for (int i=0; i<sz; i++) {
		SockIntentThread *sit = als_get(worker_sits, i);
		if (sit->worker_id == worker_id)
			return sit->sock_fd;
	}
	return ((SockIntentThread *)als_get(worker_sits, 0))->sock_fd;
}

__uint32_t d_nodes_count()
{
	// return als_size(downstream_sockets);
	log_print("[ error ] Process end: caused by an incorrect function call.\n");
	exit(EXIT_FAILURE);
	return 0;
}

ArrayList *worker_nodes() {
	return worker_sits;
}

SockIntentThread *master_sit() {
	return _master__sit_;
}