#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "log.h"
#include "command.h"
#include "utils.h"
#include "cfg.h"
#include "mdx.h"
#include "mdd.h"
#include "rb-tree.h"

extern Stack YC_STC;

// CCI - constant command intention
static EuclidCommand *CCI_ALLOW;
static EuclidCommand *CCI_CHILD_NODE_JOIN;
static EuclidCommand *CCI_TERML_CTRL;

static LinkedQueue *_ec_pool;
static pthread_mutex_t _ec_p_mtx;
// static pthread_cond_t _ec_p_cond;
static sem_t _ec_p_sem;

RedBlackTree *_thread_mam_pool;

static int command_processor_thread_startup();

// static pthread_t _command_processor_thread_id;

static void *do_process_command(void *addr);

static int execute_command(EuclidCommand *ec);

extern void *parse_mdx(char *mdx);

int init_command_module()
{
	// init CCI_ALLOW
	void *addr = obj_alloc(SZOF_USG_INT + SZOF_USG_SHORT, OBJ_TYPE__RAW_BYTES);
	*((unsigned int *)addr) = SZOF_USG_INT + SZOF_USG_SHORT;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__ALLOW;
	CCI_ALLOW = create_command(addr);

	// init CCI_CHILD_NODE_JOIN
	addr = obj_alloc(SZOF_USG_INT + SZOF_USG_SHORT, OBJ_TYPE__RAW_BYTES);
	*((unsigned int *)addr) = SZOF_USG_INT + SZOF_USG_SHORT;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__CHILD_NODE_JOIN;
	CCI_CHILD_NODE_JOIN = create_command(addr);

	// init CCI_TERML_CTRL
	addr = obj_alloc(SZOF_USG_INT + SZOF_USG_SHORT, OBJ_TYPE__RAW_BYTES);
	*((unsigned int *)addr) = SZOF_USG_INT + SZOF_USG_SHORT;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__TERMINAL_CONTROL;
	CCI_TERML_CTRL = create_command(addr);

	// init EuclidCommand pool and mutex and cond
	// init_LinkedList(&_ec_pool);
	_ec_pool = create_lnk_queue();
	pthread_mutex_init(&_ec_p_mtx, NULL);
	// pthread_cond_init(&_ec_p_cond, NULL);
	sem_init(&_ec_p_sem, 0, 0);

	_thread_mam_pool = rbt_create("MemAllocMng *", mam_comp, NULL, DIRECT, NULL);

	command_processor_thread_startup();

	return 0;
}

EuclidCommand *create_command(char *bytes)
{
	EuclidCommand *ec_p = obj_alloc(sizeof(EuclidCommand), OBJ_TYPE__EuclidCommand);
	ec_p->bytes = bytes;
	return ec_p;
}

intent ec_get_intent(EuclidCommand *ec)
{
	if (ec == NULL || ec->bytes == NULL)
		return INTENT__UNKNOWN;

	return *((intent *)((ec->bytes) + sizeof(int)));
}

// int ec_release(EuclidCommand *ec)
// {
// 	// _release_mem_(ec->bytes);
// 	// _release_mem_(ec);
// 	return 0;
// }

EuclidCommand *get_const_command_intent(intent inte)
{
	if (inte == INTENT__ALLOW)
		return CCI_ALLOW;

	if (inte == INTENT__CHILD_NODE_JOIN)
		return CCI_CHILD_NODE_JOIN;

	if (inte == INTENT__TERMINAL_CONTROL)
		return CCI_TERML_CTRL;

	return NULL;
}

int ec_get_capacity(EuclidCommand *ec)
{
	return *((int *)(ec->bytes));
}

int submit_command(EuclidCommand *ec)
{
	pthread_mutex_lock(&_ec_p_mtx);
	int res = lnk_q_add_obj(_ec_pool, ec);
	// pthread_cond_signal(&_ec_p_cond);
	pthread_mutex_unlock(&_ec_p_mtx);
	sem_post(&_ec_p_sem);
	return res;
}

static int command_processor_thread_startup()
{
	int i;
	for (i = 0; i < get_cfg()->ec_threads_count; i++)
	{
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, do_process_command, NULL);
		int detach_r = pthread_detach(thread_id);

		MemAllocMng *mam = MemAllocMng_new();
		mam->thread_id = thread_id;
		rbt_add(_thread_mam_pool, mam);

		log_print("EuclidCommand processor thread [%lu] <%d>.\n", thread_id, detach_r);
	}

	return 0;
}

static void *do_process_command(void *addr)
{
	// pthread_t thread_id = pthread_self();

	EuclidCommand *ec = NULL;
	while (1)
	{
		sem_wait(&_ec_p_sem);
		pthread_mutex_lock(&_ec_p_mtx);
		ec = (EuclidCommand *)lnk_q_get(_ec_pool);
		// while (ec == NULL)
		// {
		// 	pthread_cond_wait(&_ec_p_cond, &_ec_p_mtx);
		// 	ec = (EuclidCommand *)lnk_q_get(_ec_pool);
		// }
		pthread_mutex_unlock(&_ec_p_mtx);

		execute_command(ec);

		sem_post(&(ec->sem));

		// obj_release(ec->bytes);
		// obj_release(ec);

		MemAllocMng *mam = MemAllocMng_current_thread_mam();
		if (mam) {
			mam_reset(mam);
		}

		// ec = NULL;
	}

	return NULL;
}

static int __execute_command__count = 1;

static int execute_command(EuclidCommand *ec)
{
	log_print("@@@@@@@@@@@@@@@@@@ execute command count = %d\n", __execute_command__count++);
	intent inte = ec_get_intent(ec);
	if (inte == INTENT__INSERT_CUBE_MEARSURE_VALS)
	{
		distribute_store_measure(ec);
	}
	else if (inte == INTENT__MDX)
	{
		parse_mdx((ec->bytes) + 10);
		void *ids_type;
		stack_pop(&YC_STC, &ids_type);
		if (ids_type == IDS_STRLS_CRTDIMS)
		{
			ArrayList *dim_names_ls;
			stack_pop(&YC_STC, (void **)&dim_names_ls);
			create_dims(dim_names_ls);
		}
		else if (ids_type == IDS_STRLS_CRTMBRS)
		{
			ArrayList *mbrs_info_als;
			stack_pop(&YC_STC, (void **)&mbrs_info_als);
			create_members(mbrs_info_als);
		}
		else if (ids_type == IDS_OBJLS_BIUCUBE)
		{
			ArrayList *measures_ls, *dims_roles_ls;
			char *cube_name;
			stack_pop(&YC_STC, (void **)&measures_ls);
			stack_pop(&YC_STC, (void **)&dims_roles_ls);
			stack_pop(&YC_STC, (void **)&cube_name);
			build_cube(cube_name, dims_roles_ls, measures_ls);
		}
		else if (ids_type == IDS_CXOBJ_ISRTCUBEMEARS)
		{
			ArrayList *ls_vms;
			char *cube_name;
			stack_pop(&YC_STC, (void **)&ls_vms);
			stack_pop(&YC_STC, (void **)&cube_name);
			insert_cube_measure_vals(cube_name, ls_vms);
		}
		else if (ids_type == IDS_MULTI_DIM_SELECT_DEF)
		{
			log_print("[ INFO ] - MDX QUERY: %s\n", (ec->bytes) + 10);
			SelectDef *select_def;
			stack_pop(&YC_STC, (void **)&select_def);
			MultiDimResult *md_rs = exe_multi_dim_queries(select_def);
			log_print("// TODO ....................... %s:%d\n", __FILE__, __LINE__); // TODO should be return a multi-dim-result
			MultiDimResult_print(md_rs);
		}
		else if (ids_type == IDS_ARRLS_DIMS_LVS_INFO)
		{
			// log_print("**********************************************************************************\n");
			// log_print("**********************************************************************************\n");
			// log_print("**********************************************************************************\n");
			ArrayList *dim_lv_map_ls;
			stack_pop(&YC_STC, (void **)&dim_lv_map_ls);
			int i,j,map_len, map_count = als_size(dim_lv_map_ls);
			for (i = 0; i < map_count; i++) {
				ArrayList *map = als_get(dim_lv_map_ls, i);
				char *dimension_name = als_get(map, 0);
				Dimension *dim = find_dim_by_name(dimension_name);
				map_len = als_size(map);
				for (j=1;j<map_len;j+=2) {
					void *lv_trans = als_get(map, j);
					long *lv_p = (long *)&lv_trans;
					char *level_name = als_get(map, j + 1);

					Level *level = Level_creat(level_name, dim, *lv_p);
					mdd__save_level(level);
					mdd__use_level(level);

					// create_level(dimension_name, *lv_p, level_name);

					// if (j % 2 == 0) {
					// 	log_print("[%s]    ",ele);
					// } else {
					// 	long *lv_p = (long *)&ele;
					// 	log_print("%ld:",*lv_p);
					// }
				}
				// log_print("\n");
			}
			// log_print("**********************************************************************************\n");
			// log_print("**********************************************************************************\n");
			// log_print("**********************************************************************************\n");
		}
		else {
			log_print("[ error ] program exit(1), cause by: unknow ids_type < %p >\n", ids_type);
			exit(1);
		}
	} else {
		log_print("[ error ] program exit(1), unknown inte.\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}

EuclidCommand *build_intent_command_mdx(char *mdx)
{
	// The last byte is to hold the final null character of the string.
	unsigned int capacity = SZOF_USG_INT + SZOF_USG_SHORT + SZOF_USG_INT + strlen(mdx) + 1;

	EuclidCommand *ec = obj_alloc(sizeof(EuclidCommand), OBJ_TYPE__EuclidCommand);
	void *addr = obj_alloc(capacity, OBJ_TYPE__RAW_BYTES);

	*((unsigned int *)addr) = capacity;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__MDX;
	*((unsigned int *)(addr + SZOF_USG_INT + SZOF_USG_SHORT)) = strlen(mdx);

	memcpy(addr + SZOF_USG_INT + SZOF_USG_SHORT + SZOF_USG_INT, mdx, strlen(mdx));

	ec->bytes = addr;

	return ec;
}
