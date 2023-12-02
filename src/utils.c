#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <malloc.h>

#include "log.h"
#include "utils.h"
#include "rb-tree.h"
#include "memory-management.h"

extern RedBlackTree *_thread_mam_pool;

// todo modify 'short' to 'type_obj'
void *mam_alloc(size_t size, short type, MemAllocMng *mam, int mam_mark) {

	assert(BYTES_ALIGNMENT > sizeof(short));

	size_t required = (mam_mark ? sizeof(MemAllocMng *) : 0) + BYTES_ALIGNMENT + size;
	if (required % BYTES_ALIGNMENT != 0)
		required = (required / BYTES_ALIGNMENT + 1) * BYTES_ALIGNMENT;

	mam = mam ? mam : MemAllocMng_current_thread_mam();
	void *obj_ins;

	if ((required + sizeof(char *) + sizeof(unsigned long)) > MAM_BLOCK_MAX) {

		// log_print("@@MAM@@ -->> mam_alloc(%p) ... obj_alloc < %lu >\n", mam, sizeof(char *) + required);

		char *big_blk = obj_alloc(sizeof(char *) + required, OBJ_TYPE__RAW_BYTES);
		if (mam->big_block)
			*((char **)big_blk) = mam->big_block;
		mam->big_block = big_blk;
		char *obj_hide_head = mam->big_block + sizeof(char *);
		if (mam_mark) {
			*((MemAllocMng **)obj_hide_head) = mam;
			*((short *)(obj_hide_head + sizeof(MemAllocMng *) + BYTES_ALIGNMENT - sizeof(short))) = (0xC000) | type;
			obj_ins = obj_hide_head + sizeof(MemAllocMng *) + BYTES_ALIGNMENT;
		} else {
			*((short *)(obj_hide_head + BYTES_ALIGNMENT - sizeof(short))) = (0x8000) | type;
			obj_ins = obj_hide_head + BYTES_ALIGNMENT;
		}

		assert((((unsigned long)obj_ins) & BYTES_ALIG_CHK_MASK) == 0);
		assert(((unsigned long)obj_ins) % BYTES_ALIGNMENT == 0);
		return obj_ins;
	}

	char *blk = mam->current_block;
	unsigned long remaining_capacity = MAM_BLOCK_MAX - *((unsigned long *)(blk + sizeof(char *)));

	if (required > remaining_capacity) {

		// log_print("@@MAM@@ -->> mam_alloc(%p) ... obj_alloc < %lu > MAM_BLOCK_MAX\n", mam, MAM_BLOCK_MAX);

		blk = obj_alloc(MAM_BLOCK_MAX, OBJ_TYPE__RAW_BYTES);
		*((unsigned long *)(blk + sizeof(char *))) = sizeof(char *) + sizeof(unsigned long);
		*((char **)blk) = mam->current_block;
		mam->current_block = blk;
	}

	unsigned long index = *((unsigned long *)(blk + sizeof(char *)));

	if (mam_mark) {
		/**
		 * The memory allocation manager information and object type need to be recorded in the header hidden data.
		 */
		*((MemAllocMng **)(blk + index)) = mam;
		*((short *)(blk + index + sizeof(MemAllocMng *) + BYTES_ALIGNMENT - sizeof(short))) = (0xC000) | type;
		obj_ins = blk + index + sizeof(MemAllocMng *) + BYTES_ALIGNMENT;
	} else {
		/**
		 * The object type needs to be recorded in the header hidden data.
		 */
		*((short *)(blk + index + BYTES_ALIGNMENT - sizeof(short))) = (0x8000) | type;
		obj_ins = blk + index + BYTES_ALIGNMENT;
	}

	*((unsigned long *)(blk + sizeof(char *))) = index + required;

	assert((((unsigned long)obj_ins) & BYTES_ALIG_CHK_MASK) == 0);
	assert(((unsigned long)obj_ins) % BYTES_ALIGNMENT == 0);
	return obj_ins;
}

void *mam_hlloc(MemAllocMng *mam, size_t size) {
	assert(mam != NULL);
	size = size > BYTES_ALIGNMENT ? size - BYTES_ALIGNMENT : 0;

	char *addr = mam_alloc(size, OBJ_TYPE__RAW_BYTES, mam, 0);
	addr -= BYTES_ALIGNMENT;
	memset(addr, 0, BYTES_ALIGNMENT);

	return addr;
}

MemAllocMng *MemAllocMng_current_thread_mam() {
	MemAllocMng key;
	memset(&key, 0, sizeof(MemAllocMng));
	key.thread_id = pthread_self();
	return rbt__find(_thread_mam_pool, &key)->obj;
}

void mam_reset(MemAllocMng *mam) {
	mam->exception_desc = NULL;

	char *curr_blk = mam->current_block;
	char *next_blk = *((void **)curr_blk);

	memset(curr_blk, 0, MAM_BLOCK_MAX);

	*((unsigned long *)(curr_blk + sizeof(char *))) = sizeof(char *) + sizeof(unsigned long);

	while (next_blk) {
		curr_blk = next_blk;
		next_blk = *((void **)curr_blk);
		obj_release(curr_blk);
	}

	curr_blk = mam->big_block;
	mam->big_block = NULL;
	while (curr_blk) {
		next_blk = *((char **)curr_blk);
		obj_release(curr_blk);
		curr_blk = next_blk;
	}

	// log_print("@@MAM@@ -->> mam_reset(%p)\n", mam);
}

int mam_comp(void *mam, void *other) {
	MemAllocMng *m = mam;
	MemAllocMng *o = other;

	if ((m->id | m->thread_id == 0) || (m->id * m->thread_id != 0) || (o->id | o->thread_id == 0) || (o->id * o->thread_id != 0)) {
		log_print("[ error ] exit. cause by: exception in mam_comp(..)\n");
		exit(EXIT_FAILURE);
	}

	if (m->id * o->id)
		return o->id < m->id ? -1 : (o->id > m->id ? 1 : 0);

	if (m->thread_id * o->thread_id)
		return o->thread_id < m->thread_id ? -1 : (o->thread_id > m->thread_id ? 1 : 0);

	return o->id ? -1 : 1;
}

MemAllocMng *MemAllocMng_new() {
	MemAllocMng *mam = obj_alloc(sizeof(MemAllocMng), OBJ_TYPE__MemAllocMng);
	mam->current_block = obj_alloc(MAM_BLOCK_MAX, OBJ_TYPE__RAW_BYTES);
	*((unsigned long *)(mam->current_block + sizeof(char *))) = sizeof(char *) + sizeof(unsigned long);
	// log_print("@@MAM@@ -->> MemAllocMng_new() : %p\n", mam);
	return mam;
}

// TODO about to be deprecated, replaced by obj_info
short obj_type_of(void *obj)
{
	assert((((unsigned long)obj) & BYTES_ALIG_CHK_MASK) == 0);
	assert(((unsigned long)obj) % BYTES_ALIGNMENT == 0);

	return (*(((short *)obj) - 1)) & 0x3FFF;
}

void obj_info(void *obj, short *type, enum obj_mem_alloc_strategy *strat, MemAllocMng **mp) {

	assert((((unsigned long)obj) & BYTES_ALIG_CHK_MASK) == 0);
	assert(((unsigned long)obj) % BYTES_ALIGNMENT == 0);

	*type = (*(((short *)obj) - 1)) & 0x3FFF;
	*mp = NULL;
	switch ((*(((short *)obj) - 1)) & 0xC000)
	{
		case 0x0000:
			*strat = DIRECT;
			break;
		case 0xC000:
			*mp = *((MemAllocMng **)(obj - BYTES_ALIGNMENT - sizeof(MemAllocMng *)));
		case 0x8000:
			*strat = USED_MAM;
			break;
		default:
			log_print("[ error ] program exit! cause by: exception in obj_info(...)\n");
			exit(1);
	}
}

MemAllocMng *obj_mam(void *obj) {
	short type;
	enum_oms strat;
	MemAllocMng *mam;
	obj_info(obj, &type, &strat, &mam);
	return mam;
}

void *obj_alloc(size_t size, short type) {
	size += BYTES_ALIGNMENT;
	char *obj_head = allocate_memory(size);
	// log_print("@@obj_alloc@@ << %p %lu\n", obj_head, size);
	memset(obj_head, 0, size);
	// *obj_head = type;
	*(short *)(obj_head + BYTES_ALIGNMENT - sizeof(short)) = type;
	assert((((unsigned long)(obj_head + BYTES_ALIGNMENT)) & BYTES_ALIG_CHK_MASK) == 0);
	assert(((unsigned long)(obj_head + BYTES_ALIGNMENT)) % BYTES_ALIGNMENT == 0);
	return obj_head + BYTES_ALIGNMENT;
}

void obj_release(void *obj) {
	assert((((unsigned long)obj) & BYTES_ALIG_CHK_MASK) == 0);
	assert(((unsigned long)obj) % BYTES_ALIGNMENT == 0);

	if ( *(((short *)obj)-1) < 0 ) {
		log_print("[ error ] - Program exit! Cause by: this memory cannot be freed here.\n");
		exit(1);
	}
	char *freed = ((char *)obj) - BYTES_ALIGNMENT;
	// log_print("@@obj_release@@ >> %p\n", freed);
	release_memory(freed);
}

ssize_t read_sock_pkg(int sock_fd, void **buf, size_t *buf_len)
{
	int pkg_capacity;
	int bs_count;			  // count of bytes read at a time
	int buf_cursor = 0;		  // It also indicates the total number of bytes that have been read
	int remaining_retry = 10; // Retry attempts remaining

	do
	{
		bs_count = read(sock_fd, ((void *)&pkg_capacity) + buf_cursor, sizeof(pkg_capacity) - buf_cursor);

		if (bs_count < 0 || (bs_count == 0 && remaining_retry-- == 0))
			return -1;

		buf_cursor += bs_count;

	} while (buf_cursor < sizeof(pkg_capacity));

	// If the packet is too large or too small, the memory of the response is not allocated, but the failure is returned quickly.
	if ((pkg_capacity <= sizeof(pkg_capacity)) || (pkg_capacity > (16 * 1024 * 1024)))
		return -1;

	// Memory needs to be freed manually.
	*buf = (char *)obj_alloc(pkg_capacity, OBJ_TYPE__RAW_BYTES);

	memcpy(*buf, &pkg_capacity, sizeof(pkg_capacity));

	remaining_retry = 10;
	while (buf_cursor < pkg_capacity)
	{
		bs_count = read(sock_fd, *buf + buf_cursor, pkg_capacity - buf_cursor);
		if (bs_count) { // bs_count != 0
			buf_cursor += bs_count;
		} else { // bs_count == 0
			remaining_retry--;
			if (remaining_retry == 0) {
				obj_release(*buf);
				*buf = NULL;
				return -1;
			}
		}
	}
// printf("READ PACKAGE DATA >>>>>> %12d %12d\n", *(int *)*buf, *(short *)((*buf) + sizeof(int)));

	return *buf_len = pkg_capacity;
}

LinkedQueue *create_lnk_queue()
{
	return obj_alloc(sizeof(LinkedQueue), OBJ_TYPE__LinkedQueue);
}

int lnk_q_add_obj(LinkedQueue *lnk_q, void *obj)
{
	LinkedQueueNode *n = obj_alloc(sizeof(LinkedQueueNode), OBJ_TYPE__LinkedQueueNode);
	n->obj = obj;

	if (lnk_q->tail)
	{
		(lnk_q->tail)->next = n;
		lnk_q->tail = n;
	}
	else
	{
		lnk_q->tail = lnk_q->head = n;
	}

	return 0;
}

void *lnk_q_get(LinkedQueue *lnk_q)
{
	if (lnk_q->head == NULL)
		return NULL;

	LinkedQueueNode *cn = lnk_q->head;
	if (lnk_q->head == lnk_q->tail)
	{
		lnk_q->head = lnk_q->tail = NULL;
	}
	else
	{
		lnk_q->head = cn->next;
	}

	void *res = cn->obj;
	obj_release(cn);
	return res;
}

int sock_conn_to(int *sock_fd, char *ip, int port)
{
	*sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(ip);

	if (connect(*sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0)
		return 0;

	log_print("Failed to connect [ %s:%d ]\n", ip, port);

	return -1;
}

StrArr *str_split(char *orig_str, char *deli_str)
{
	int len = 1;
	char *p = orig_str;

	while (p = strstr(p, deli_str))
	{
		p += strlen(deli_str);
		len++;
	}

	StrArr *str_arr = obj_alloc(sizeof(StrArr), OBJ_TYPE__StrArr);
	str_arr->length = len;

	str_arr->head_str_p = obj_alloc(sizeof(void *) * len + 1 + strlen(orig_str), OBJ_TYPE__RAW_BYTES);

	strcpy(((char *)str_arr->head_str_p) + sizeof(void *) * len, orig_str);
	p = ((char *)str_arr->head_str_p) + sizeof(void *) * len;

	len = 1;
	*(str_arr->head_str_p) = p;

	while (p = strstr(p, deli_str))
	{
		*p = '\0';
		p += strlen(deli_str);
		*(str_arr->head_str_p + len) = p;
		len++;
	}

	// show_StrArr(str_arr);
	return str_arr;
}

void destory_StrArr(StrArr *arr_address)
{
	obj_release(arr_address->head_str_p);
	obj_release(arr_address);
}

void show_StrArr(StrArr *arr)
{
	log_print("\n");
	int i;
	for (i = 0; i < arr->length; i++)
	{
		log_print("%d\t%s\n", i, *(arr->head_str_p + i));
	}
	log_print("\n");
}

char *str_arr_get(StrArr *sa, unsigned int i)
{
	if (sa == NULL || i >= sa->length)
		return NULL;

	return *((sa->head_str_p) + i);
}

int stack_push(Stack *s, void *obj)
{
	assert(s != NULL && s->top_idx < STACK_MAX_DEEP);

	// if (s == NULL || s->top_idx == STACK_MAX_DEEP)
	// 	return -1;

	(s->bucket)[(s->top_idx)++] = obj;

	return 0;
}

int stack_pop(Stack *s, void **addr)
{
	assert(s != NULL && s->top_idx > 0);

	// if (s == NULL || s->top_idx == 0)
	// 	return -1;

	*addr = (s->bucket)[--(s->top_idx)];

	return 0;
}

void stack_reset(Stack *stk) {
	stk->top_idx = 0;
}

// char *str_clone(char *str)
// {
// 	if (str == NULL)
// 		return NULL;

// 	char *str_b_cl = (char *)obj_alloc(strlen(str) + 1, OBJ_TYPE__RAW_BYTES);

// 	strcpy(str_b_cl, str);

// 	return str_b_cl;
// }

int open_serv_sock(int *ss_fd_p, int port)
{
	int ss_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (ss_fd < 0)
	{
		log_print("error at fn:open_serv_sock\n");
		return -1;
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(ss_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		goto failed_close_exit;

	if (listen(ss_fd, SOMAXCONN) < 0)
		goto failed_close_exit;

	*ss_fd_p = ss_fd;
	return 0;

failed_close_exit:
	log_print("failed at fn:open_serv_sock.\n");
	if (close(ss_fd) != 0)
		log_print("failed at fn:open_serv_sock.\n");

	return -1;
}

long now_microseconds()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

int append_file_data(char *file_path, char *data, size_t sz)
{
	// char f[128];
	// memset(f, 0, 128);
	// getcwd(f, 80);
	// if (*file_path != '/')
	// 	strcat(f, "/");
	// strcat(f, file_path);

	FILE *fp = fopen(file_path, "a+");
	fwrite(data, sz, 1, fp);
	fclose(fp);
	return 0;
}

// FILE *open_file(char *r_path, char *modes)
// {
// 	char f[128];
// 	memset(f, 0, 128);
// 	getcwd(f, 80);
// 	if (*r_path != '/')
// 		strcat(f, "/");
// 	strcat(f, r_path);
// 	return fopen(f, "a+");
// }

int append_file_uint(char *file_path, __uint32_t val)
{
	return append_file_data(file_path, (char *)&val, sizeof(val));
}

ArrayList *als_new(unsigned int init_capacity, char *desc, enum_oms strat, MemAllocMng *mam) {

	ArrayList *als;

	if (strat == DIRECT) {
		als = obj_alloc(sizeof(ArrayList), OBJ_TYPE__ArrayList);
		als->elements_arr_p = obj_alloc(sizeof(void *) * init_capacity, OBJ_TYPE__RAW_BYTES);
	} else {
		// mam = mam ? mam : MemAllocMng_current_thread_mam();
		mam = strat == SPEC_MAM ? mam : MemAllocMng_current_thread_mam();
		als = mam_alloc(sizeof(ArrayList), OBJ_TYPE__ArrayList, mam, 1);
		als->elements_arr_p = mam_alloc(sizeof(void *) * init_capacity, OBJ_TYPE__RAW_BYTES, mam, 0);
	}

	als->ele_arr_capacity = init_capacity;

	if (desc != NULL)
	{
		int desc_len = strlen(desc);
		desc_len = desc_len < COMMON_OBJ_DESC_LEN ? desc_len : COMMON_OBJ_DESC_LEN - 1;
		memcpy(als->desc, desc, desc_len);
	}

	return als;
}

void als_sync(ArrayList *als) {
	assert(als != NULL && als->sync_lock == NULL);

	short type;
	enum_oms strat;
	MemAllocMng *mam;
	obj_info(als, &type, &strat, &mam);

	if (strat == DIRECT) {
		als->sync_lock = obj_alloc(sizeof(pthread_mutex_t), OBJ_TYPE__RAW_BYTES);
	} else if (mam) {
		als->sync_lock = mam_alloc(sizeof(pthread_mutex_t), OBJ_TYPE__RAW_BYTES, mam, 0);
	} else {
		log_print("[ error ] <Fail fast!!!> Mistake in function 'void als_sync(ArrayList *als)'.\n");
		exit(EXIT_FAILURE);
	}

	pthread_mutex_init(als->sync_lock, NULL);
}

void als_release(ArrayList *arr_ls) {

	short type;
	enum_oms strat;
	MemAllocMng *mam;
	obj_info(arr_ls, &type, &strat, &mam);

	if (arr_ls->sync_lock)
		pthread_mutex_destroy(arr_ls->sync_lock);

	if (strat == DIRECT) {
		if (arr_ls->sync_lock)
			obj_release(arr_ls->sync_lock);

		obj_release(arr_ls->elements_arr_p);
		obj_release(arr_ls);
		return;
	}

	log_print("[ error ] <Fail fast!!!> Mistake in function 'void als_release(ArrayList *arr_ls)'.\n");
	exit(EXIT_FAILURE);
}

void *ArrayList_set(ArrayList *ls, unsigned int index, void *obj) {

	if (index >= ls->idx)
		return NULL;

	void *replaced = ls->elements_arr_p[index];
	ls->elements_arr_p[index] = obj;

	return replaced;
}

int als_add(ArrayList *als, void *obj)
{
	if (als == NULL)
		return -1;

	if (als->idx >= als->ele_arr_capacity)
	{
		short _als_type;
		enum obj_mem_alloc_strategy strat;
		MemAllocMng *als_mam;
		obj_info(als, &_als_type, &strat, &als_mam);

		als->ele_arr_capacity += 16;

		void **new_ele_arr_p;
		if (strat == DIRECT)
			new_ele_arr_p = obj_alloc(sizeof(void *) * (als->ele_arr_capacity), OBJ_TYPE__RAW_BYTES);
		else
			new_ele_arr_p = mam_alloc(sizeof(void *) * (als->ele_arr_capacity), OBJ_TYPE__RAW_BYTES, als_mam, 0);

		memcpy(new_ele_arr_p, als->elements_arr_p, als->idx * sizeof(void *));

		if (strat == DIRECT)
			obj_release(als->elements_arr_p);

		als->elements_arr_p = new_ele_arr_p;
	}

	(als->elements_arr_p)[als->idx++] = obj;

	return 0;
}

int als_add_sync(ArrayList *als, void *obj) {
	pthread_mutex_lock(als->sync_lock);
	int res = als_add(als, obj);
	pthread_mutex_unlock(als->sync_lock);
	return res;
}

void *als_get(ArrayList *als, unsigned int position)
{
	return position < als->idx ? als->elements_arr_p[position] : NULL;
}

unsigned int als_size(ArrayList *als)
{
	return als->idx;
}

// TODO deprecated, there may be bugs
int als_remove(ArrayList *als, void *obj)
{
	for (int i = 0; i < als->idx; i++)
	{
		if (*((int *)(als->elements_arr_p[i])) != *((int *)obj))
			continue;

		als->idx--;
		for (; i < als->idx; i++)
		{
			als->elements_arr_p[i] = als->elements_arr_p[i + 1];
		}
		return 0;
	}
	log_print("INFO - als_remove ... no object < %p >\n", obj);
	return 0;
}

int als_remove_sync(ArrayList *als, void *obj) {
	pthread_mutex_lock(als->sync_lock);
	int res = als_remove(als, obj);
	pthread_mutex_unlock(als->sync_lock);
	return res;
}

void *als_rm_index(ArrayList *als, unsigned int idx) {

	if (idx >= als->idx)
		return NULL;


	void *obj = als->elements_arr_p[idx];
	for (int i = idx; i < als->idx - 1; i++)
		als->elements_arr_p[i] = als->elements_arr_p[i + 1];

	als->idx--;

	return obj;
}

void *slide_over_mem(void *addr, ssize_t range, size_t *idx)
{
	size_t _idx = *idx;
	*idx += range;
	return addr + _idx;
}

// TODO Modified to quick sort.
void ArrayList_sort(ArrayList *ls, int (*obj_cmp_fn)(void *obj, void *other))
{
	if ((!ls) || (!obj_cmp_fn))
		return;

	int i, j, sz = als_size(ls);
	for (i = 0; i < sz - 1; i++)
	{
		for (j = i + 1; j < sz; j++)
		{
			if (obj_cmp_fn(ls->elements_arr_p[i], ls->elements_arr_p[j]) < 0)
			{
				void *tmp = ls->elements_arr_p[i];
				ls->elements_arr_p[i] = ls->elements_arr_p[j];
				ls->elements_arr_p[j] = tmp;
			}
		}
	}
}

extern pthread_mutex_t gloc__mem_counter_mtx;
static size_t used_mem_size = 0;



// char *bytes_alloc(size_t size) {

// 	return malloc(size);

// 	// char *mem_addr = malloc(size);

// 	// size_t muz = malloc_usable_size(mem_addr);

// 	// // todo replaced by __sync_bool_compare_and_swap or __sync_val_compare_and_swap
// 	// pthread_mutex_lock(&gloc__mem_counter_mtx);
// 	// size_t ums_snapshoot = used_mem_size;
// 	// used_mem_size += muz;
// 	// pthread_mutex_unlock(&gloc__mem_counter_mtx);

// 	// log_print("[ Mem statistics + ] %lu ... +%lu<%p> ... %lu\n", ums_snapshoot, muz, mem_addr, ums_snapshoot + muz);

// 	// return mem_addr;
// }



// void bytes_free(void *freed) {

// 	// size_t muz = malloc_usable_size(freed);

// 	// // todo replaced by __sync_bool_compare_and_swap or __sync_val_compare_and_swap
// 	// pthread_mutex_lock(&gloc__mem_counter_mtx);
// 	// size_t ums_snapshoot = used_mem_size;
// 	// used_mem_size -= muz;
// 	// pthread_mutex_unlock(&gloc__mem_counter_mtx);

// 	// log_print("[ Mem statistics - ] %lu ... -%lu<%p> ... %lu\n", ums_snapshoot, muz, freed, ums_snapshoot - muz);

// 	free(freed);
// }