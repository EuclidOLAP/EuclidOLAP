#ifndef EUCLID__UTILS_H
#define EUCLID__UTILS_H 1

#include <stdlib.h>
#include <stdio.h>
// #include <sys/stat.h>

#include "obj-type-def.h"

#define SZOF_INT (sizeof(int))
#define SZOF_USG_INT (sizeof(unsigned int))
#define SZOF_SHORT (sizeof(short))
#define SZOF_USG_SHORT (sizeof(unsigned short))

#define COMMON_OBJ_DESC_LEN 256

enum obj_mem_alloc_strategy {
	// Memory is allocated directly by the operating system.
	DIRECT,
	// Allocate memory in the thread memory allocation manager.
	THREAD_MAM,
	// Use the specified memory allocation manager.
	SPEC_MAM,
	// Indicates that an object uses the memory allocation manager to allocate memory.
	USED_MAM
};

typedef enum obj_mem_alloc_strategy enum_oms;

#define MAM_BLOCK_MAX (0x01UL<<20)

struct memory_allocation_manager
{
	pthread_t thread_id;

	/**
	 * Points to the data block that is allocating memory.
	 * 
	 * memory block structure:
	 * 0th to  7th bytes (void *)        - Pointer to the next block of data.
	 * 8th to 15th bytes (unsigned long) - The index of the currently allocated memory location.
	 * the rest of the bytes             - Memory for allocation.
	 */
	char *current_block;
};

typedef struct memory_allocation_manager MemAllocMng;

/**
 * @param mam If mam is NULL, use the memory allocation manager of current thread.
 * @param mam_mark When it is not 0, the address of the memory allocation manager is recorded in
 * 				   the object header information, and when it is 0, it is not recorded.
 */
void *mam_alloc(size_t size, short type, MemAllocMng *mam, int mam_mark);

int mam_comp(void *mam, void *other);

MemAllocMng *MemAllocMng_new();

MemAllocMng *MemAllocMng_current_thread_mam();

void mam_reset(MemAllocMng *);

// TODO about to be deprecated
void *__objAlloc__(size_t size, short type);

// void _release_mem_(void *obj);

// TODO about to be deprecated, replaced by obj_info
short obj_type_of(void *obj);

/**
 * @param obj
 * @param type
 * @param strat
 * @param mp
 */
void obj_info(void *obj, short *type, enum obj_mem_alloc_strategy *strat, MemAllocMng **mp);

/**
 * Directly call the operating system interface to allocate memory,
 * and the memory allocated using this function needs to be released manually.
 */
void *obj_alloc(size_t size, short type);

/**
 * Free up manually managed memory.
 */
void obj_release(void *obj);

typedef struct _linked_queue_node_
{
	void *obj;
	struct _linked_queue_node_ *next;
} LinkedQueueNode;

typedef struct _linked_queue_
{
	LinkedQueueNode *head;
	LinkedQueueNode *tail;
} LinkedQueue;

LinkedQueue *create_lnk_queue();

int lnk_q_add_obj(LinkedQueue *lnk_q, void *obj);

void *lnk_q_get(LinkedQueue *lnk_q);

typedef struct string_array
{
	char **head_str_p;
	unsigned int length;
} StrArr;

#define STACK_MAX_DEEP 256

typedef struct _struct_stack_
{
	int top_idx;
	void *bucket[STACK_MAX_DEEP];
} Stack;
int stack_push(Stack *s, void *obj);
int stack_pop(Stack *s, void **addr);

StrArr *str_split(char *orig_str, char *deli_str);

void destory_StrArr(StrArr *arr_address);

void show_StrArr(StrArr *arr);

char *str_arr_get(StrArr *sa, unsigned int i);

ssize_t read_sock_pkg(int sock_fd, void **buf, size_t *buf_len);

int open_serv_sock(int *ss_fd_p, int port);

int sock_conn_to(int *sock_fd, char *ip, int port);

char *str_clone(char *str);

long now_microseconds();

int append_file_data(char *file_path, char *data, size_t sz);

int append_file_uint(char *file_path, __uint32_t val);

// #define SIM_POOL_DESC_LEN 32

// typedef struct _simple_pool_
// {
// 	unsigned int capacity;
// 	void **handle;
// 	char desc[SIM_POOL_DESC_LEN];
// } SimplePool;

// SimplePool *simpool_create(unsigned int capacity, char *desc);

// int simpool_add(SimplePool *sp, void *obj);

typedef struct _array_list_
{
	unsigned int idx;
	unsigned int ele_arr_capacity;
	void **elements_arr_p;
	char desc[COMMON_OBJ_DESC_LEN];
} ArrayList;

// TODO deprecated - replaced by als_new
ArrayList *als_create(unsigned int init_capacity, char *desc);

/**
 * @param strat Memory allocation strategy.
 * @param mam When the strat parameter is SPEC_MAM, mam cannot be NULL.
 */
ArrayList *als_new(unsigned int init_capacity, char *desc, enum obj_mem_alloc_strategy strat, MemAllocMng *mam);

void *ArrayList_set(ArrayList *, unsigned int, void *);

// when `other` is sorted before `obj`, return -1
void ArrayList_sort(ArrayList *ls, int (*obj_cmp_fn)(void *obj, void *other));

int als_add(ArrayList *als, void *obj);

void *als_get(ArrayList *als, unsigned int position);

unsigned int als_size(ArrayList *als);

int als_remove(ArrayList *als, void *obj);

void *slide_over_mem(void *addr, ssize_t range, size_t *idx);

FILE *open_file(char *r_path, char *modes);

// void file_stat(char *data_file, struct stat *f_stat);

#endif
