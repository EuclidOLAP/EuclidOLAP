#include <assert.h>
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
#include "vce.h"
#include "net.h"
#include "obj-type-def.h"

// extern Stack AST_STACK;

// CCI - constant command intention
static EuclidCommand *CCI_ALLOW;
static EuclidCommand *CCI_WORKER_JOINS_CLUSTER;
static EuclidCommand *CCI_TERML_CTRL;
static EuclidCommand *CCI_GENERIC_SUCCESS;

static LinkedQueue *_ec_pool;
static pthread_mutex_t _ec_p_mtx;
// static pthread_cond_t _ec_p_cond;
static sem_t _ec_p_sem;

RedBlackTree *_thread_mam_pool;

static int command_processor_thread_startup();

// static pthread_t _command_processor_thread_id;

static void *do_process_command(void *addr);

static int execute_command(EuclidCommand *ec);

// extern void *parse_mdx(char *mdx);

int init_command_module()
{
	size_t _sz_ = 0;
	char *__ptr = NULL;
	EuclidConfig *cfg = get_cfg();

	// init CCI_ALLOW
	char *addr = obj_alloc(SZOF_USG_INT + SZOF_USG_SHORT, OBJ_TYPE__RAW_BYTES);
	*((unsigned int *)addr) = SZOF_USG_INT + SZOF_USG_SHORT;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__ALLOW;
	CCI_ALLOW = create_command(addr);

	// init CCI_WORKER_JOINS_CLUSTER
	_sz_ = SZOF_USG_INT + SZOF_USG_SHORT + sizeof(unsigned long);
	addr = obj_alloc(_sz_, OBJ_TYPE__RAW_BYTES);
	*((unsigned int *)addr) = _sz_;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__WORKER_JOINS_CLUSTER;
	*((unsigned long *)(addr + SZOF_USG_INT + SZOF_USG_SHORT)) = strtoul(cfg->worker_id, &__ptr, 10);
	CCI_WORKER_JOINS_CLUSTER = create_command(addr);

	// init CCI_TERML_CTRL
	addr = obj_alloc(SZOF_USG_INT + SZOF_USG_SHORT, OBJ_TYPE__RAW_BYTES);
	*((unsigned int *)addr) = SZOF_USG_INT + SZOF_USG_SHORT;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__TERMINAL_CONTROL;
	CCI_TERML_CTRL = create_command(addr);

	// init CCI_GENERIC_SUCCESS
	addr = obj_alloc(SZOF_USG_INT + SZOF_USG_SHORT + strlen("successful") + 1, OBJ_TYPE__RAW_BYTES);
	*((unsigned int *)addr) = SZOF_USG_INT + SZOF_USG_SHORT + strlen("successful") + 1;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__SUCCESSFUL;
	strcpy(addr + SZOF_USG_INT + SZOF_USG_SHORT, "successful");
	CCI_GENERIC_SUCCESS = create_command(addr);

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
	assert(ec != NULL);
	assert(ec->bytes != NULL);

	return *(intent *)(ec->bytes + sizeof(int));
}

EuclidCommand *get_const_command_intent(intent inte)
{
	if (inte == INTENT__ALLOW)
		return CCI_ALLOW;

	if (inte == INTENT__WORKER_JOINS_CLUSTER)
		return CCI_WORKER_JOINS_CLUSTER;

	if (inte == INTENT__TERMINAL_CONTROL)
		return CCI_TERML_CTRL;

	if (inte == INTENT__SUCCESSFUL)
		return CCI_GENERIC_SUCCESS;

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

		pthread_mutex_unlock(&_ec_p_mtx);

		MemAllocMng *mam = MemAllocMng_current_thread_mam();
		mam_reset(mam);

		if (setjmp(mam->excep_ctx_env) == 0)
		{
			int exe_result = execute_command(ec);
			if (ec->result == NULL)
			{
				if (mam->exception_desc)
				{
					ec->result = ec_new(INTENT__EXE_RESULT_DESC, strlen(mam->exception_desc) + 1);
					strcpy(ec->result->bytes + SZOF_INT + SZOF_SHORT, mam->exception_desc);
				}
				else if (exe_result != 0)
				{
					// task execution failed
					ec->result = EuclidCommand_failure("Unrecognized statement.");
				}
			}
		}
		else
		{
			ec->result = ec_new(INTENT__EXE_RESULT_DESC, strlen(mam->exception_desc) + 1);
			strcpy(ec->result->bytes + SZOF_INT + SZOF_SHORT, mam->exception_desc);
		}

		sem_post(&(ec->sem));
	}

	return NULL;
}

static int __execute_command__count = 1;

static int execute_command(EuclidCommand *action)
{
	Stack stk;

	intent inte = ec_get_intent(action);
	if (inte == INTENT__INSERT_CUBE_MEASURE_VALS)
	{
		// This line of code is executed only when running in worker mode.
		assert(get_cfg()->mode == MODE_WORKER);
		distribute_store_measure(action, 0);
	}
	else if (inte == INTENT__MDX /* || inte == INTENT__MDX_EXPECT_RESULT_TXT */)
	{
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		// Set the MDX parsing completion flag to 0.
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFE;

		parse_mdx((action->bytes) + 10, &stk);

		if ((cur_thrd_mam->bin_flags & 0x0001) == 0)
		{
			// Empty the stack to prevent stack overflow.
			stk.top_idx = 0;

			// The MDX expression was not parsed.
			return -1;
		}

		void *ids_type;

		if (stack_pop(&stk, &ids_type) != 0)
		{
			log_print("[ error ] Program exit. Impossible program execution location.\n");
			exit(EXIT_FAILURE);
		}

		if (ids_type == IDS_STRLS_CRTDIMS)
		{
			ArrayList *dim_names = NULL;
			stack_pop(&stk, (void **)&dim_names);
			ArrayList *def_hie_names = NULL;
			stack_pop(&stk, (void **)&def_hie_names);
			create_dims(dim_names, def_hie_names, &(action->result));
		} else if (ids_type == IDS_CREATE_HIERARCHY) {
			char *dimension_name = NULL;
			char *hierarchy_name = NULL;
			stack_pop(&stk, (void **)&hierarchy_name);
			stack_pop(&stk, (void **)&dimension_name);
			create_hierarchy(find_dim_by_name(dimension_name), hierarchy_name);
		}
		else if (ids_type == IDS_STRLS_CRTMBRS)
		{
			ArrayList *mbrs_info_als;
			stack_pop(&stk, (void **)&mbrs_info_als);
			create_members(mbrs_info_als);
		}
		else if (ids_type == IDS_OBJLS_BIUCUBE)
		{
			ArrayList *measures_ls, *dims_roles_ls;
			char *cube_name;
			stack_pop(&stk, (void **)&measures_ls);
			stack_pop(&stk, (void **)&dims_roles_ls);
			stack_pop(&stk, (void **)&cube_name);
			return build_cube(cube_name, dims_roles_ls, measures_ls);
		}
		else if (ids_type == IDS_CXOBJ_ISRTCUBEMEARS)
		{
			ArrayList *ls_vms = NULL;
			unsigned long worker_id = 0;
			char *cube_name = NULL;
			stack_pop(&stk, (void **)&ls_vms);
			stack_pop(&stk, (void **)&worker_id);
			stack_pop(&stk, (void **)&cube_name);
			insert_cube_measure_vals(cube_name, ls_vms, worker_id);
		}
		else if (ids_type == IDS_MULTI_DIM_SELECT_DEF)
		{
			// log_print("[ INFO ] - MDX QUERY: %s\n", (ec->bytes) + 10);
			SelectDef *select_def;
			stack_pop(&stk, (void **)&select_def);

			// Check if the axes of the result set are repeatedly defined.
			for (int i = 0; i < als_size(select_def->ax_def_ls); i++)
			{
				for (int j = 0; j != i && j < als_size(select_def->ax_def_ls); j++)
				{
					AxisDef *axi = als_get(select_def->ax_def_ls, i);
					AxisDef *axj = als_get(select_def->ax_def_ls, j);
					if (axi->posi == axj->posi)
					{
						cur_thrd_mam->exception_desc = "exception: There are multiple axes with duplicate positions in the query results.";
						longjmp(cur_thrd_mam->excep_ctx_env, -1);
					}
				}
			}

			MultiDimResult *md_rs = exe_multi_dim_queries(select_def);

			if (cur_thrd_mam->exception_desc == NULL)
			{
				ByteBuf *binuf = mdrs_to_bin(md_rs);
				char *payload = obj_alloc(binuf->index, OBJ_TYPE__RAW_BYTES);
				memcpy(payload, binuf->buf_addr, binuf->index);
				buf_release(binuf);
				action->result = create_command(payload);
			}
		}
		else if (ids_type == IDS_ARRLS_DIMS_LVS_INFO)
		{
			ArrayList *dim_lv_map_ls;
			stack_pop(&stk, (void **)&dim_lv_map_ls);

			// check for unknown dimensions
			for (int i = 0; i < als_size(dim_lv_map_ls); i++)
			{
				ArrayList *map = als_get(dim_lv_map_ls, i);
				char *dimension_name = als_get(map, 0);
				if (find_dim_by_name(dimension_name))
					continue;
				cur_thrd_mam->exception_desc = "exception: nonexistent dimension.";
				return 0;
			}

			int i, j, map_len, map_count = als_size(dim_lv_map_ls);
			for (i = 0; i < map_count; i++)
			{
				ArrayList *map = als_get(dim_lv_map_ls, i);
				char *dimension_name = als_get(map, 0);
				char *hierarchy_name = als_get(map, 1);
				Dimension *dim = find_dim_by_name(dimension_name);

				Hierarchy *hierarchy = hierarchy_name ? dim_find_hierarchy_by_name(dim, hierarchy_name) : find_hierarchy(dim->def_hierarchy_gid);

				map_len = als_size(map);
				for (j = 2; j < map_len; j += 2)
				{
					void *lv_trans = als_get(map, j);
					long *lv_p = (long *)&lv_trans;
					char *level_name = als_get(map, j + 1);
					Level *level = Level_creat(level_name, dim, hierarchy, *lv_p);
					mdd__save_level(level);
					mdd__use_level(level);
				}
			}
		} else if (ids_type == IDS_MAKE_EQUIVALENT) {
			ArrayList *up_list = NULL;
			stack_pop(&stk, (void **)&up_list);
			for (int i = 0; i < als_size(up_list); i += 2) {
				MDMEntityUniversalPath *src_up = als_get(up_list, i);
				MDMEntityUniversalPath *dest_up = als_get(up_list, i + 1);

				Member *ms = up_evolving(NULL, src_up, NULL, NULL);
				assert(obj_type_of(ms) == OBJ_TYPE__Member);
				Member *md = up_evolving(NULL, dest_up, NULL, NULL);
				assert(obj_type_of(md) == OBJ_TYPE__Member);

				ms->link = md->gid;
				append_file_data(get_cfg()->profiles.members, (char *)ms, sizeof(Member));
			}
		}
		else
		{
			log_print("[ error ] program exit(1), cause by: unknow ids_type < %p >\n", ids_type);
			exit(1);
		}
	}
	else if (inte == INTENT__VECTOR_AGGREGATION)
	{

		ArrayList *grids = worker_aggregate_measure(action);

		long len = grids ? als_size(grids) : 0;

		int payload_capacity = 4 + 2 + 8 + 8 + 4 + 4 + 8;
		payload_capacity += len * (sizeof(double) + sizeof(char));

		char *payload = obj_alloc(payload_capacity, OBJ_TYPE__RAW_BYTES);
		char *idx = payload;

		*((int *)idx) = payload_capacity;
		idx += sizeof(payload_capacity);

		short inte = INTENT__AGGREGATE_TASK_RESULT;
		*((short *)idx) = inte;
		idx += sizeof(inte);

		memcpy(idx, action->bytes + sizeof(int) + sizeof(short), 8 + 8 + 4 + 4);
		idx += 8 + 8 + 4 + 4;
		*((long *)idx) = len;
		idx += sizeof(len);

		for (int i = 0; i < len; i++)
		{
			GridData *gd = als_get(grids, i);

			if (gd != NULL)
				*((double *)idx) = gd->val;

			idx += sizeof(double);
		}

		for (int i = 0; i < len; i++)
		{
			GridData *gd = als_get(grids, i);

			*idx = gd ? gd->null_flag : 1;
			idx += sizeof(char);
		}

		assert(action->result == NULL);

		action->result = create_command(payload);
	}
	else if (inte == INTENT__AGGREGATE_TASK_RESULT)
	{
		log_print("// todo at once ............................ INTENT__AGGREGATE_TASK_RESULT\n");
	}
	else if (inte == INTENT__SUCCESSFUL)
	{
		log_print("// todo at once ............................ INTENT__SUCCESSFUL\n");
	}
	else
	{
		log_print("[ error ] program exit(1), unknown inte < %d >\n", inte);
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

EuclidCommand *EuclidCommand_failure(char *desc)
{
	assert(desc != NULL);
	assert(strlen(desc) > 0);

	int len = SZOF_INT + SZOF_SHORT + strlen(desc) + 1;
	char *payload = obj_alloc(len, OBJ_TYPE__RAW_BYTES);
	*(int *)payload = len;
	*(intent *)(payload + SZOF_INT) = INTENT__FAILURE;
	strcpy(payload + SZOF_INT + SZOF_SHORT, desc);
	return create_command(payload);
}

EuclidCommand *ec_new(intent inte, size_t payload_sz)
{
	EuclidCommand *ec = obj_alloc(sizeof(EuclidCommand), OBJ_TYPE__EuclidCommand);
	ec->bytes = obj_alloc(SZOF_USG_INT + SZOF_USG_SHORT + payload_sz, OBJ_TYPE__RAW_BYTES);
	*((unsigned int *)ec->bytes) = SZOF_USG_INT + SZOF_USG_SHORT + payload_sz;
	*((intent *)(ec->bytes + SZOF_USG_INT)) = inte;
	return ec;
}

void ec_change_intent(EuclidCommand *ec, intent inte)
{
	*(intent *)(ec->bytes + sizeof(int)) = inte;
}