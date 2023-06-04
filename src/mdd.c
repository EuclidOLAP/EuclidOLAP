#include <assert.h>
#include <stdio.h>
#include <string.h>
// #include <dirent.h>
#include <unistd.h> // for usleep
#include <pthread.h>

#include "log.h"
#include "mdd.h"
#include "mdx.h"
#include "command.h"
#include "cfg.h"
#include "net.h"
#include "vce.h"
#include "utils.h"
#include "obj-type-def.h"

// extern Stack AST_STACK;

// extern void *parse_mdx(char *mdx);

static md_gid lastest_md_gid = -1;

static ArrayList *dims_pool = NULL;
static ArrayList *member_pool = NULL;
static ArrayList *cubes_pool = NULL;
static ArrayList *levels_pool = NULL;

static MemAllocMng *meta_mam;

static ArrayList *agg_tasks_pool;
static pthread_mutex_t agg_tasks_lock;

static Member *_create_member_lv1(Dimension *dim, char *mbr_name);
static Member *_create_member_child(Member *parent, char *child_name);

static ArrayList *select_def__build_axes(MDContext *md_ctx, SelectDef *);

// static Cube *select_def__get_cube(SelectDef *);

static MddTuple *cube__basic_ref_vector(Cube *);

static MddTuple *ax_def__head_ref_tuple(MDContext *md_ctx, AxisDef *, MddTuple *, Cube *);

static MddTuple *tuple__merge(MddTuple *cxt_tuple, MddTuple *tuple_frag);

static MddAxis *ax_def__build(MDContext *md_ctx, AxisDef *, MddTuple *, Cube *);

static unsigned int mdd_ax__len(MddAxis *);

static unsigned int mdd_set__len(MddSet *);

static MddTuple *ids_tupledef__build(MDContext *md_ctx, TupleDef *t_def, MddTuple *context_tuple, Cube *cube);

int mdd_init()
{
	meta_mam = MemAllocMng_new();

	dims_pool = als_new(32, "dimensions pool", SPEC_MAM, meta_mam);
	member_pool = als_new(256, "members pool | Member *", SPEC_MAM, meta_mam);
	cubes_pool = als_new(8, "cubes pool", SPEC_MAM, meta_mam);
	levels_pool = als_new(128, "Level *", SPEC_MAM, meta_mam);

	agg_tasks_pool = als_new(32, "ArrayList *", SPEC_MAM, meta_mam);
	pthread_mutex_init(&agg_tasks_lock, NULL);
}

static int load_dimensions()
{

	FILE *dims_file = open_file(META_DEF_DIMS_FILE_PATH, "r");
	Dimension dim;
	while (1)
	{
		if (fread(&dim, sizeof(Dimension), 1, dims_file) < 1)
			break;

		Dimension *dimension = mam_alloc(sizeof(Dimension), OBJ_TYPE__Dimension, meta_mam, 0);
		memcpy(dimension, &dim, sizeof(Dimension));
		als_add(dims_pool, dimension);
	}
	return fclose(dims_file);
}

static int load_levels()
{
	FILE *levels_file = open_file(META_DEF_LEVELS_FILE_PATH, "r");
	Level level;
	while (1)
	{
		if (fread(&level, sizeof(Level), 1, levels_file) < 1)
			break;

		Level *lv = mam_alloc(sizeof(Level), OBJ_TYPE__Level, meta_mam, 0);
		memcpy(lv, &level, sizeof(Level));
		als_add(levels_pool, lv);
	}
	return fclose(levels_file);
}

static int load_members()
{
	FILE *members_file = open_file(META_DEF_MBRS_FILE_PATH, "r");
	Member memb;
	while (1)
	{
		if (fread(&memb, sizeof(Member), 1, members_file) < 1)
			break;

		Member *member = mam_alloc(sizeof(Member), OBJ_TYPE__Member, meta_mam, 0);
		memcpy(member, &memb, sizeof(Member));

		/* TODO
		 * In order to eliminate the duplicate data in the dimension member file, a bad method is used here,
		 * which needs to be solved in the subsequent optimization.
		 */
		int i;
		for (i = 0; i < als_size(member_pool); i++)
		{
			Member *m_existed = als_get(member_pool, i);
			if (m_existed->gid == member->gid)
			{
				ArrayList_set(member_pool, i, member);
				goto goto_flag;
			}
		}
		als_add(member_pool, member);
	goto_flag:
		member = member;
	}

	fclose(members_file);

	int i, sz = als_size(member_pool);
	for (i = 0; i < sz; i++)
		mdd__gen_mbr_abs_path(als_get(member_pool, i));

	return 0;
}

static int load_cubes()
{

	// char c_file[128];
	// memset(c_file, 0, 128);
	// getcwd(c_file, 80);
	// strcat(c_file, META_DEF_CUBES_FILE_PATH);

	FILE *cubes_fd = open_file(META_DEF_CUBES_FILE_PATH, "r");

	char cube_stru_file[128];
	md_gid cube_id;

	while (fread((void *)&cube_id, sizeof(md_gid), 1, cubes_fd) > 0)
	{
		// als_add(cube_id_arr, *((void **)&cube_id));
		memset(cube_stru_file, 0, 128);
		sprintf(cube_stru_file, "/meta/cube_%lu", cube_id);

		FILE *cube_fd = open_file(cube_stru_file, "r");

		Cube *cube = mam_alloc(sizeof(Cube), OBJ_TYPE__Cube, meta_mam, 0);
		fread(cube, sizeof(Cube), 1, cube_fd);
		cube->dim_role_ls = als_new(24, "DimensionRole *", SPEC_MAM, meta_mam);
		cube->measure_mbrs = als_new(12, "Member *", SPEC_MAM, meta_mam);

		int i, dr_count;
		fread(&dr_count, sizeof(unsigned int), 1, cube_fd);
		for (i = 0; i < dr_count; i++)
		{
			DimensionRole *dim_role = mam_alloc(sizeof(DimensionRole), OBJ_TYPE__DimensionRole, meta_mam, 0);
			fread(dim_role, sizeof(DimensionRole), 1, cube_fd);
			als_add(cube->dim_role_ls, dim_role);
		}

		Dimension mea_dim__;
		fread(&mea_dim__, sizeof(Dimension), 1, cube_fd);
		cube->measure_dim = find_dim_by_gid(mea_dim__.gid);

		int mea_mbrs_count;
		fread(&mea_mbrs_count, sizeof(unsigned int), 1, cube_fd);
		for (i = 0; i < mea_mbrs_count; i++)
		{
			Member *mea_mbr = mam_alloc(sizeof(Member), OBJ_TYPE__Member, meta_mam, 0);
			fread(mea_mbr, sizeof(Member), 1, cube_fd);
			mdd__gen_mbr_abs_path(mea_mbr);
			als_add(cube->measure_mbrs, mea_mbr);
		}

		als_add(cubes_pool, cube);

		fclose(cube_fd);
	}
	fclose(cubes_fd);
}

int mdd_load()
{
	load_dimensions();
	load_levels();
	load_members();
	load_cubes();
}

Member *_new_member(char *name, md_gid dim_gid, md_gid parent_gid, __u_short lv);

Dimension *create_dimension(char *dim_name)
{
	if (strlen(dim_name) >= MD_ENTITY_NAME_BYTSZ)
	{
		log_print("[WARN] - dim name too long <%s>\n", dim_name);
		return NULL;
	}

	// 1 - create a dimension object.
	Dimension *dim = mam_alloc(sizeof(Dimension), OBJ_TYPE__Dimension, meta_mam, 0);
	dim->gid = gen_md_gid();
	memcpy(dim->name, dim_name, strlen(dim_name));
	log_print("[INFO] create dimension [ %ld ] %s\n", dim->gid, dim->name);

	// 2 - save the dim-obj into a persistent file.
	append_file_data(META_DEF_DIMS_FILE_PATH, (char *)dim, sizeof(Dimension));

	// create a root level of dimension
	Level *rootLv = Level_creat("ROOT_LEVEL", dim, 0);
	mdd__save_level(rootLv);
	mdd__use_level(rootLv);

	als_add(dims_pool, dim);
	return dim;
}

int create_dims(ArrayList *dim_names, EuclidCommand **result)
{
	__uint32_t i, sz = als_size(dim_names);
	ArrayList *dimensions_exist = als_new(sz, "char *", THREAD_MAM, NULL);

	for (i = 0; i < sz; i++)
	{
		char *dim_name = (char *)als_get(dim_names, i);

		if (find_dim_by_name(dim_name) == NULL)
			create_dimension(dim_name);
		else
			als_add(dimensions_exist, dim_name);
	}

	if (als_size(dimensions_exist) > 0)
	{
		*result = ec_new(INTENT__EXE_RESULT_DESC, 128 + als_size(dimensions_exist) * 70);

		char *desc = (*result)->bytes + SZOF_INT + SZOF_SHORT;
		sprintf(desc, "Dimensions");
		desc += strlen(desc);
		for (int i = 0; i < als_size(dimensions_exist); i++)
		{
			char *exist_dim_name = als_get(dimensions_exist, i);
			if (strlen(exist_dim_name) > 64)
			{
				*desc = ' ';
				desc++;
				memcpy(desc, exist_dim_name, 61);
				desc += 61;
				sprintf(desc, "...,");
				desc += strlen(desc);
			}
			else
			{
				sprintf(desc, " %s,", exist_dim_name);
				desc += strlen(desc);
			}
		}
		desc--;
		sprintf(desc, " are not created because dimensions with the same names already exist.");
	}

	return 0;
}

md_gid gen_md_gid()
{
	while (1)
	{
		long microseconds = now_microseconds();
		if (microseconds > lastest_md_gid)
			return lastest_md_gid = microseconds;
		usleep(2);
	}
}

Level *Level_creat(char *name_, Dimension *dim, unsigned int level_)
{

	if (strlen(name_) >= MD_ENTITY_NAME_BYTSZ)
		return NULL;

	Level *lv = mam_alloc(sizeof(Level), OBJ_TYPE__Level, meta_mam, 0);
	lv->gid = gen_md_gid();
	memcpy(lv->name, name_, strlen(name_));
	lv->dim_gid = dim->gid;
	lv->level = level_;

	return lv;
}

void mdd__save_level(Level *lv)
{
	append_file_data(META_DEF_LEVELS_FILE_PATH, (char *)lv, sizeof(Level));
}

void mdd__use_level(Level *lv)
{
	als_add(levels_pool, lv);
}

Member *create_member(ArrayList *mbr_path)
{
	unsigned int sz = als_size(mbr_path);
	int i = 0;
	int new_leaf_mbr_lv = sz - 1;

	Dimension *dim = find_dim_by_name(als_get(mbr_path, 0));
	if (dim == NULL)
		dim = create_dimension(als_get(mbr_path, 0));

	Member *mbr_lv1 = find_member_lv1(dim, als_get(mbr_path, 1));
	if (mbr_lv1 == NULL)
	{
		mbr_lv1 = _create_member_lv1(dim, als_get(mbr_path, 1));
		append_file_data(META_DEF_MBRS_FILE_PATH, (char *)mbr_lv1, sizeof(Member));

		als_add(member_pool, mbr_lv1);
	}

	if (mbr_lv1->lv < new_leaf_mbr_lv && mdd_mbr__is_leaf(mbr_lv1))
	{
		mdd_mbr__set_as_leaf(mbr_lv1);
		append_file_data(META_DEF_MBRS_FILE_PATH, (char *)mbr_lv1, sizeof(Member));
	}

	Member *p_m = mbr_lv1;
	Member *m = mbr_lv1;
	for (i = 2; i < sz; i++)
	{
		m = find_member_child(p_m, als_get(mbr_path, i));
		if (m == NULL)
		{
			m = _create_member_child(p_m, als_get(mbr_path, i));
			append_file_data(META_DEF_MBRS_FILE_PATH, (char *)m, sizeof(Member));
			als_add(member_pool, m);
		}

		if (m->lv < new_leaf_mbr_lv && mdd_mbr__is_leaf(m))
		{
			mdd_mbr__set_as_leaf(m);
			append_file_data(META_DEF_MBRS_FILE_PATH, (char *)m, sizeof(Member));
		}

		p_m = m;
	}

	return m;
}

int create_members(ArrayList *mbrs_info_als)
{
	unsigned int size = als_size(mbrs_info_als);
	int i = 0;
	while (i < size)
	{
		create_member(als_get(mbrs_info_als, i++));
	}
	return 0;
}

Dimension *find_dim_by_name(char *dim_name)
{
	unsigned int i = 0, sz = als_size(dims_pool);
	while (i < sz)
	{
		Dimension *dim = als_get(dims_pool, i++);
		if (strcmp(dim_name, dim->name) == 0)
			return dim;
	}

	return NULL;
}

Dimension *find_dim_by_gid(md_gid dim_gid)
{
	unsigned int i = 0, sz = als_size(dims_pool);
	while (i < sz)
	{
		Dimension *dim = als_get(dims_pool, i++);
		if (dim->gid == dim_gid)
			return dim;
	}

	return NULL;
}

Member *find_member_lv1(Dimension *dim, char *mbr_name)
{
	__uint32_t i = 0, sz = als_size(member_pool);
	while (i < sz)
	{
		Member *mbr = als_get(member_pool, i++);
		if ((strcmp(mbr_name, mbr->name) == 0) && dim->gid == mbr->dim_gid)
			return mbr;
	}
	return NULL;
}

Member *Member_same_lv_m(Member *member, int offset)
{
	int i, curr_m_idx, m_pool_sz = als_size(member_pool);
	ArrayList *same_lv_ms = als_new(128, "Member *", DIRECT, NULL);
	for (i = 0; i < m_pool_sz; i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->dim_gid == member->dim_gid && m->lv == member->lv)
			als_add(same_lv_ms, m);
		if (m->gid == member->gid)
			curr_m_idx = als_size(same_lv_ms) - 1;
	}

	Member *result = NULL;

	int sibling_idx = curr_m_idx + offset;
	if (sibling_idx >= 0 && sibling_idx < als_size(same_lv_ms))
		result = als_get(same_lv_ms, sibling_idx);

	als_release(same_lv_ms);

	return result;
}

Member *Member_get_posi_child(Member *parent, int child_posi)
{
	int i, m_pool_sz = als_size(member_pool);

	ArrayList *children = als_new(128, "Member *", DIRECT, NULL);

	for (i = 0; i < m_pool_sz; i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->p_gid == parent->gid)
			als_add(children, m);
	}

	Member *_child_ = child_posi >= 0 && child_posi < als_size(children) ? als_get(children, child_posi) : NULL;

	als_release(children);

	return _child_;
}

Member *Member_find_ancestor(Member *member, unsigned int distance)
{
	if (member->lv < 1 + distance) // equivalent member->lv - distance <= 0
		return NULL;
	int i;
	for (i = 0; i < distance; i++)
	{
		member = find_member_by_gid(member->p_gid);
	}
	return member;
}

ArrayList *Member_descendant_position(Member *ancestor, Member *descendant)
{
	ArrayList *posi = als_new(32, "long", THREAD_MAM, NULL);
	int i, distance = descendant->lv - ancestor->lv;
	for (i = 0; i < distance; i++)
	{
		als_add(posi, NULL);
	}
	for (i = distance - 1; i >= 0; i--)
	{
		Member *parent = find_member_by_gid(descendant->p_gid);
		long child_posi = Member_child_position(parent, descendant);
		ArrayList_set(posi, i, *((void **)&child_posi));
		descendant = parent;
	}
	return posi;
}

ArrayList *Member__descendants(Member *ancestor)
{
	int i, sz = als_size(member_pool);
	ArrayList *descendants = als_new(128, "Member *", THREAD_MAM, NULL);
	for (i = 0; i < sz; i++)
	{
		Member *member = als_get(member_pool, i);
		if (member->lv <= ancestor->lv)
			continue;
		Member *m_ance = Member_find_ancestor(member, member->lv - ancestor->lv);
		if (m_ance->gid == ancestor->gid)
			als_add(descendants, member);
	}
	return descendants;
}

Member *Member_find_posi_descmbr(Member *ancestor, ArrayList *desc_posi)
{
	int i, sz = als_size(desc_posi);
	Member *descmbr = ancestor;
	for (i = 0; i < sz; i++)
	{
		void *addr2long = als_get(desc_posi, i);
		descmbr = Member_get_posi_child(descmbr, *((long *)&addr2long));
	}
	return descmbr;
}

// TODO parameter 'parent' may be redundant
int Member_child_position(Member *parent, Member *child)
{
	int i, posi = 0, sz = als_size(member_pool);

	for (i = 0; i < sz; i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->p_gid == parent->gid)
		{
			if (m->gid == child->gid)
				return posi;
			++posi;
		}
	}

	return -1;
}

Member *find_member_child(Member *parent_mbr, char *child_name)
{
	__uint32_t i = 0, sz = als_size(member_pool);
	while (i < sz)
	{
		Member *mbr = als_get(member_pool, i++);
		if ((strcmp(child_name, mbr->name) == 0) && parent_mbr->gid == mbr->p_gid)
			return mbr;
	}
	return NULL;
}

static Member *_create_member_lv1(Dimension *dim, char *mbr_name)
{
	return _new_member(mbr_name, dim->gid, 0, 1);
}

static Member *_create_member_child(Member *parent, char *child_name)
{
	return _new_member(child_name, parent->dim_gid, parent->gid, parent->lv + 1);
}

Member *_new_member(char *name, md_gid dim_gid, md_gid parent_gid, __u_short lv)
{
	if (strlen(name) >= MD_ENTITY_NAME_BYTSZ)
		return NULL;

	Member *mbr = mam_alloc(sizeof(Member), OBJ_TYPE__Member, meta_mam, 0);
	memcpy(mbr->name, name, strlen(name));
	mbr->gid = gen_md_gid();
	mbr->dim_gid = dim_gid;
	mbr->p_gid = parent_gid;
	mbr->lv = lv;
	log_print("[INFO] new Member - dim_gid [ %ld ] p_gid [% 17ld ] gid [ %ld ] name [ %s ] lv [ %d ]\n", mbr->dim_gid, mbr->p_gid, mbr->gid, mbr->name, mbr->lv);

	return mbr;
}

int build_cube(char *name, ArrayList *dim_role_ls, ArrayList *measures)
{
	if (strlen(name) >= MD_ENTITY_NAME_BYTSZ)
		return -1;

	// Create a cube object.
	Cube *cube = mam_alloc(sizeof(Cube), OBJ_TYPE__Cube, meta_mam, 0);
	memcpy(cube->name, name, strlen(name));
	cube->gid = gen_md_gid();
	cube->dim_role_ls = als_new(24, "DimensionRole *", SPEC_MAM, meta_mam);
	cube->measure_mbrs = als_new(12, "Member *", SPEC_MAM, meta_mam);
	log_print("[INFO] new Cube - gid [ %ld ] name [ %s ]\n", cube->gid, cube->name);

	// Create several dimensional role objects and associate them to the cube.
	size_t i, dr_sz = als_size(dim_role_ls);
	for (i = 0; i < dr_sz; i += 2)
	{
		char *dim_name = als_get(dim_role_ls, i);

		// Check if non-existing dimension is associated.
		if (find_dim_by_name(dim_name) == NULL)
		{
			MemAllocMng_current_thread_mam()->exception_desc = "This cube is associated with dimensions that do not exist.";
			return -1;
		}

		char *dim_role_name = als_get(dim_role_ls, i + 1);
		Dimension *dim = find_dim_by_name(dim_name);

		DimensionRole *d_role = mam_alloc(sizeof(DimensionRole), OBJ_TYPE__DimensionRole, meta_mam, 0);
		d_role->sn = i / 2;
		memcpy(d_role->name, dim_role_name, strlen(dim_role_name));
		d_role->gid = gen_md_gid();
		d_role->cube_gid = cube->gid;
		d_role->dim_gid = dim->gid;
		log_print("[INFO] new DimensionRole - Cube [ %ld % 16s ] Dim [ %ld % 16s ] DR [ %ld % 16s ]\n",
				  cube->gid, cube->name, dim->gid, dim->name, d_role->gid, d_role->name);

		als_add(cube->dim_role_ls, d_role);
	}

	// Create a measure dimension object.
	Dimension *mear_dim = create_dimension(STANDARD_MEASURE_DIMENSION);

	cube->measure_dim = mear_dim;

	// Create several measure dimension members.
	size_t mea_sz = als_size(measures);
	for (i = 0; i < mea_sz; i++)
	{
		Member *mea_mbr = _new_member(als_get(measures, i), mear_dim->gid, 0, 1);
		als_add(cube->measure_mbrs, mea_mbr);
	}

	// Each cube uses a persistent file separately.
	char cube_file[128];
	sprintf(cube_file, "/meta/cube_%lu", cube->gid);
	append_file_data(cube_file, (char *)cube, sizeof(Cube));
	append_file_uint(cube_file, als_size(cube->dim_role_ls));
	__uint32_t r_sz = als_size(cube->dim_role_ls);
	for (i = 0; i < r_sz; i++)
	{
		DimensionRole *_d_r = als_get(cube->dim_role_ls, i);
		append_file_data(cube_file, (char *)_d_r, sizeof(DimensionRole));
	}
	append_file_data(cube_file, (char *)cube->measure_dim, sizeof(Dimension));
	append_file_uint(cube_file, als_size(cube->measure_mbrs));
	__uint32_t mm_sz = als_size(cube->measure_mbrs);
	for (i = 0; i < mm_sz; i++)
	{
		Member *mea_mbr = als_get(cube->measure_mbrs, i);
		append_file_data(cube_file, (char *)mea_mbr, sizeof(Member));
	}

	append_file_data(META_DEF_CUBES_FILE_PATH, (char *)&(cube->gid), sizeof(cube->gid));
	als_add(cubes_pool, cube);

	return 0;
}

// Measure values will be stored in local memory and disk.
int store_measure(EuclidCommand *ec)
{
	// Store in the current node.
	return vce_append(ec);
}

int distribute_store_measure(EuclidCommand *ec)
{
	EuclidConfig *cfg = get_cfg();

	if (cfg->mode != MODE_MASTER) {
		// Store in the current node.
		return store_measure(ec);
	}


	// if (d_nodes_count() < 1 || rand() % 2)
	// {
	// 	return store_measure(ec); // Store in the current node.
	// }

	return send(random_child_sock(), ec->bytes, *((int *)(ec->bytes)), 0) == (ssize_t)(*((int *)(ec->bytes)));
}

int insert_cube_measure_vals(char *cube_name, ArrayList *ls_ids_vctr_mear)
{
	ByteBuf *buf = buf__alloc(64 * 1024);

	buf_cutting(buf, sizeof(int));
	*((unsigned short *)buf_cutting(buf, sizeof(short))) = INTENT__INSERT_CUBE_MEASURE_VALS;

	Cube *cube = find_cube_by_name(cube_name);
	if (cube == NULL)
	{
		MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
		thrd_mam->exception_desc = "exception: An undefined cube was encountered.";
		longjmp(thrd_mam->excep_ctx_env, -1);
	}

	*(md_gid *)buf_cutting(buf, sizeof(md_gid)) = cube->gid;

	*(unsigned int *)buf_cutting(buf, sizeof(int)) = als_size(cube->dim_role_ls);

	*(unsigned int *)buf_cutting(buf, sizeof(int)) = als_size(cube->measure_mbrs);

	unsigned int j, k, sz = als_size(ls_ids_vctr_mear);

	for (unsigned int i = 0; i < sz; i++)
	{
		IDSVectorMears *ids_vm = als_get(ls_ids_vctr_mear, i);
		unsigned int vct_sz = als_size(ids_vm->ls_vector);

		if (vct_sz != als_size(cube->dim_role_ls))
		{
			buf_release(buf);
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: Insert statement does not match cube.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		for (j = 0; j < vct_sz; j++)
		{
			ArrayList *mbr_path_str = als_get(ids_vm->ls_vector, j);
			size_t ap_bsz = sizeof(int) + sizeof(md_gid) * (als_size(mbr_path_str) - 1);

			// gen_member_gid_abs_path(cube, mbr_path_str, buf_cutting(buf, ap_bsz));
			if (gen_member_gid_abs_path(cube, mbr_path_str, buf_cutting(buf, ap_bsz)) != 0) {
				buf_release(buf);
				MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
				thrd_mam->exception_desc = "exception: A member name found cannot be matched.";
				longjmp(thrd_mam->excep_ctx_env, -1);
			}
		}

		unsigned int cube_mmbrs_sz = als_size(cube->measure_mbrs);

		unsigned int mv_sz = als_size(ids_vm->ls_mears_vals);

		for (j = 0; j < cube_mmbrs_sz; j++)
		{
			Member *mm = als_get(cube->measure_mbrs, j);

			// Set the null-value flag bit, 1 means the measure-value is null.
			buf_cutting(buf, sizeof(double));
			*(char *)buf_cutting(buf, sizeof(char)) = 1;

			for (k = 0; k < mv_sz; k++)
			{
				char *mm_name = als_get(ids_vm->ls_mears_vals, k);
				if (strcmp(mm_name, mm->name) != 0)
					continue;

				char *cursor = buf_cursor(buf);
				cursor -= sizeof(char);
				*cursor = 0;
				cursor -= sizeof(double);
				*(double *)cursor = *(double *)(als_get(ids_vm->ls_mears_vals, k + 1));

				break;
			}
		}
	}

	// set data package capacity
	*(unsigned int *)buf_starting(buf) = buf_size(buf);

	char *payload = obj_alloc(buf_size(buf), OBJ_TYPE__RAW_BYTES);
	memcpy(payload, buf_starting(buf), buf_size(buf));
	buf_release(buf);

	EuclidCommand *_ec_ = create_command(payload);

	// Store measure values locally or distribute it to downstream nodes for processing
	int res = distribute_store_measure(_ec_);

	obj_release(_ec_->bytes);
	obj_release(_ec_);

	return res;
}

Cube *find_cube_by_name(char *cube_name)
{
	__uint32_t i, sz = als_size(cubes_pool);
	for (i = 0; i < sz; i++)
	{
		Cube *cube = als_get(cubes_pool, i);
		if (strcmp(cube_name, cube->name) == 0)
			return cube;
	}
	return NULL;
}

Cube *find_cube_by_gid(md_gid id)
{
	__uint32_t i, sz = als_size(cubes_pool);
	for (i = 0; i < sz; i++)
	{
		Cube *cube = als_get(cubes_pool, i);
		if (cube->gid == id)
			return cube;
	}
	return NULL;
}

/**
 * @return 0 - normal; not 0 - mistake
 */
int gen_member_gid_abs_path(Cube *cube, ArrayList *mbr_path_str, char *abs_path)
{
	char *dim_role_name = als_get(mbr_path_str, 0);
	DimensionRole *dr;
	Dimension *dim;
	Member *lv1_mbr, *mbr;
	unsigned int i, num_drs = als_size(cube->dim_role_ls);
	for (i = 0; i < num_drs; i++)
	{
		dr = als_get(cube->dim_role_ls, i);
		if (strcmp(dim_role_name, dr->name) != 0)
			continue;
		dim = find_dim_by_gid(dr->dim_gid);
		mbr = lv1_mbr = find_member_lv1(dim, (char *)als_get(mbr_path_str, 1));
		break;
	}

	if (lv1_mbr == NULL)
		return -1;

	unsigned int sz = als_size(mbr_path_str);

	*((unsigned int *)abs_path) = sz - 1;
	*((md_gid *)(abs_path + sizeof(unsigned int))) = lv1_mbr->gid;

	for (i = 2; i < sz; i++)
	{
		mbr = find_member_child(mbr, als_get(mbr_path_str, i));
		if (mbr == NULL)
			return -1;

		*((md_gid *)(abs_path + sizeof(unsigned int) + sizeof(md_gid) * (i - 1))) = mbr->gid;
	}

	return 0;
}

static long query_times = 1;

MultiDimResult *exe_multi_dim_queries(SelectDef *select_def)
{
	// log_print("\n[ debug ] >>>>>>>>>>>>>>>>>>>>>>> The number of times the query was executed: %ld\n\n", query_times++);

	MemAllocMng *th_mam = MemAllocMng_current_thread_mam();

	if (select_def__get_cube(select_def) == NULL)
	{
		th_mam->exception_desc = "exception: nonexistent cube.";
		longjmp(th_mam->excep_ctx_env, -1);
	}

	MDContext *md_ctx = MDContext_creat();
	md_ctx->select_def = select_def;

	// Build the real axes in this multidimensional query.
	ArrayList *axes = select_def__build_axes(md_ctx, select_def);

	int i;
	for (i = 0; i < als_size(axes); i++)
	{
		MddAxis *ax = als_get(axes, i);
		if (als_size(ax->set->tuples) == 0)
			return NULL;
	}

	ArrayList_sort(axes, MddAxis_cmp);
	unsigned int x_size = als_size(axes);

	// Cross these axes to generate result set.
	unsigned long rs_len = 1;

	for (i = 0; i < x_size; i++)
	{
		MddAxis *ax = als_get(axes, i);
		rs_len *= mdd_ax__len(ax);
	}

	unsigned int offset_arr[x_size];
	offset_arr[x_size - 1] = 1;

	for (i = x_size - 2; i >= 0; i--)
		offset_arr[i] = offset_arr[i + 1] * mdd_ax__len(als_get(axes, i + 1));

	MddTuple **tuples_matrix_h = mam_alloc(rs_len * x_size * sizeof(void *), OBJ_TYPE__RAW_BYTES, NULL, 0);

	int matx_col, matx_row, f;
	for (matx_col = 0; matx_col < x_size; matx_col++)
	{
		matx_row = 0;
		MddAxis *ax = als_get(axes, matx_col);
		while (matx_row < rs_len)
		{
			for (i = 0; i < mdd_ax__len(ax); i++)
			{
				MddTuple *tuple = mdd_ax__get_tuple(ax, i);
				for (f = 0; f < offset_arr[matx_col]; f++)
					tuples_matrix_h[(matx_row++) * x_size + matx_col] = tuple;
			}
		}
	}

	Cube *cube = select_def__get_cube(select_def);

	MddTuple *basic_tuple = cube__basic_ref_vector(cube);
	if (select_def->where_tuple_def)
	{
		MddTuple *where_tuple = ids_tupledef__build(md_ctx, select_def->where_tuple_def, basic_tuple, cube);
		basic_tuple = tuple__merge(basic_tuple, where_tuple);
	}

	for (i = 0; i < rs_len; i++)
	{
		tuples_matrix_h[i] = _MddTuple__mergeTuples(tuples_matrix_h + (i * x_size), x_size);
		tuples_matrix_h[i] = tuple__merge(basic_tuple, tuples_matrix_h[i]);
	}

	MultiDimResult *md_result = MultiDimResult_creat();

	EuclidConfig *cfg = get_cfg();
	if (cfg->mode == MODE_MASTER) {
		ArrayList *direct_vectors = als_new(rs_len, "MddTuple *", THREAD_MAM, NULL);
		ArrayList *calcul_vectors = als_new(rs_len, "MddTuple *", THREAD_MAM, NULL);

		ArrayList *__merge_in__ = als_new(rs_len, "void *", THREAD_MAM, NULL);

		void *__dv__ = NULL;
		void *__cv__ = &__dv__;

		for (i = 0; i < rs_len; i++) {
			tuples_matrix_h[i]->attachment = i;
			if (tup_is_calculated(tuples_matrix_h[i])) {
				als_add(direct_vectors, tuples_matrix_h[i]);
				als_add(__merge_in__, __dv__);
			} else {
				als_add(calcul_vectors, tuples_matrix_h[i]);
				als_add(__merge_in__, __cv__);
			}
		}

		double *md_rs__vals;
		char *md_rs__null_flags;
		unsigned long md_rs__rs_len;

		if (als_size(direct_vectors) > 0)
			dispatchAggregateMeasure(/*md_ctx,*/ cube, direct_vectors, &md_rs__vals, &md_rs__null_flags, &md_rs__rs_len);


		ArrayList *cal_grids = als_new(als_size(calcul_vectors), "GridData *", THREAD_MAM, NULL);

		if (als_size(calcul_vectors) > 0) {
			// GridData grid_data;
			for (int k=0;k<als_size(calcul_vectors);k++) {
				MddTuple *cal_tp = als_get(calcul_vectors, k);
				unsigned int csz__ = als_size(cal_tp->mr_ls);

				GridData *gd = mam_hlloc(MemAllocMng_current_thread_mam(), sizeof(GridData));

				for (int i__ = csz__ - 1; i__ >= 0; i__--)
				{
					MddMemberRole *mr = als_get(cal_tp->mr_ls, i__);
					if (mr->member_formula)
					{
						Expression *exp = mr->member_formula->exp;
						Expression_evaluate(md_ctx, exp, cube, cal_tp, gd);
						break;
					}
				}

				als_add(cal_grids, gd);
			}
		}

		md_result->vals = mam_alloc(sizeof(double) * rs_len, OBJ_TYPE__RAW_BYTES, NULL, 0);
		md_result->null_flags = mam_alloc(sizeof(char) * rs_len, OBJ_TYPE__RAW_BYTES, NULL, 0);
		md_result->rs_len = rs_len;

		int di = 0, ci = 0;
		for (int x=0; x<rs_len; x++) {
			if (als_get(__merge_in__, x) == __dv__) {
				md_result->vals[x] = md_rs__vals[di];
				md_result->null_flags[x] = md_rs__null_flags[di];
				++di;
			} else {
				GridData *gd = als_get(cal_grids, ci++);
				md_result->vals[x] = gd->val;
				md_result->null_flags[x] = gd->null_flag;
			}
		}

		md_result->axes = axes;

		return md_result;
	}

	// 'measure_vals' is equivalent to a double array whose length is 'rs_len'.
	double *measure_vals = vce_vactors_values(md_ctx, tuples_matrix_h, rs_len, &(md_result->null_flags));
	md_result->axes = axes;
	md_result->vals = measure_vals;
	md_result->rs_len = rs_len;

	return md_result;
}

static ArrayList *select_def__build_axes(MDContext *md_ctx, SelectDef *select_def)
{
	ArrayList *ax_def_ls = select_def->ax_def_ls;
	Cube *cube = select_def__get_cube(select_def);
	MddTuple *ref_tuple = cube__basic_ref_vector(cube);

	if (select_def->where_tuple_def)
	{
		MddTuple *where_tuple = ids_tupledef__build(md_ctx, select_def->where_tuple_def, ref_tuple, cube);
		ref_tuple = tuple__merge(ref_tuple, where_tuple);
	}

	int ax_count = als_size(ax_def_ls);
	int i, j;
	for (i = 0; i < ax_count; i++)
	{
		for (j = 0; j < ax_count; j++)
		{
			AxisDef *ax_def = als_get(ax_def_ls, j);
			MddTuple *ax_head_ref_tuple = ax_def__head_ref_tuple(md_ctx, ax_def, ref_tuple, cube);

			ref_tuple = tuple__merge(ref_tuple, ax_head_ref_tuple);
		}
	}
	ArrayList *axes_ls = als_new(16, "MddAxis *", THREAD_MAM, NULL);
	for (i = 0; i < ax_count; i++)
	{
		AxisDef *ax_def = als_get(ax_def_ls, i);
		MddAxis *ax = ax_def__build(md_ctx, ax_def, ref_tuple, cube);
		als_add(axes_ls, ax);
	}

	return axes_ls;
}

/*static*/ Cube *select_def__get_cube(SelectDef *select_def)
{
	int i;
	int cubes_count = als_size(cubes_pool);
	Cube *cube;
	for (i = 0; i < cubes_count; i++)
	{
		cube = als_get(cubes_pool, i);
		if (strcmp(cube->name, select_def->cube_def->name) == 0)
			return cube;
	}
	return NULL;
}

static MddTuple *cube__basic_ref_vector(Cube *cube)
{
	MddTuple *tuple = mdd_tp__create();
	int i, j;
	int r_count = als_size(cube->dim_role_ls);
	for (i = 0; i < r_count; i++)
	{
		DimensionRole *dim_role = als_get(cube->dim_role_ls, i);
		int mp_size = als_size(member_pool);
		for (j = 0; j < mp_size; j++)
		{
			Member *mbr = als_get(member_pool, j);
			if (mbr->dim_gid == dim_role->dim_gid && (mbr->lv == 1) && (strcmp(mbr->name, "ALL") == 0))
			{
				MddMemberRole *mbr_role = mdd_mr__create(mbr, dim_role);
				mdd_tp__add_mbrole(tuple, mbr_role);
				break;
			}
		}
	}

	MddMemberRole *measure_mr = mdd_mr__create(als_get(cube->measure_mbrs, 0), NULL);
	mdd_tp__add_mbrole(tuple, measure_mr);
	return tuple;
}

static MddTuple *ax_def__head_ref_tuple(MDContext *md_ctx, AxisDef *ax_def, MddTuple *t, Cube *c)
{
	return ids_setdef__head_ref_tuple(md_ctx, ax_def->set_def, t, c);
}

/*
 * ctx_tuple:  [Goods].[Transport].[starting region].[ending region].[starting date].[completion date].[**MeasureDimRole**]
 * tuple_frag: [Transport].[completion date].[Goods].[starting region].[ending region]
 *
 * result:     [starting date].[**MeasureDimRole**].[Transport].[completion date].[Goods].[starting region].[ending region]
 */
static MddTuple *tuple__merge(MddTuple *ctx_tuple, MddTuple *tuple_frag)
{
	if (tuple_frag == NULL)
		return ctx_tuple;

	unsigned int ctx_sz = als_size(ctx_tuple->mr_ls);
	unsigned int frag_sz = als_size(tuple_frag->mr_ls);

	MddTuple *tp = mdd_tp__create();
	int i, j;

	for (i = 0; i < ctx_sz; i++)
	{
		MddMemberRole *ctx_mr = (MddMemberRole *)als_get(ctx_tuple->mr_ls, i);
		for (j = 0; j < frag_sz; j++)
		{
			MddMemberRole *f_mr = (MddMemberRole *)als_get(tuple_frag->mr_ls, j);

			if ((ctx_mr->dim_role != NULL && f_mr->dim_role != NULL) && (ctx_mr->dim_role->gid == f_mr->dim_role->gid))
			{
				goto jump_a;
			}

			if (ctx_mr->dim_role == NULL && f_mr->dim_role == NULL)
			{
				goto jump_a;
			}
		}
		mdd_tp__add_mbrole(tp, ctx_mr);
	jump_a:
		i = i;
	}

	for (j = 0; j < frag_sz; j++)
	{
		MddMemberRole *f_mr = (MddMemberRole *)als_get(tuple_frag->mr_ls, j);
		mdd_tp__add_mbrole(tp, f_mr);
	}

	// for (i = 0; i < ctx_sz; i++)
	// {
	// 	MddMemberRole *ctx_mr = (MddMemberRole *)als_get(ctx_tuple->mr_ls, i);
	// 	for (j = 0; j < frag_sz; j++)
	// 	{
	// 		MddMemberRole *f_mr = (MddMemberRole *)als_get(tuple_frag->mr_ls, j);

	// 		if ((ctx_mr->dim_role != NULL && f_mr->dim_role != NULL) && (ctx_mr->dim_role->gid == f_mr->dim_role->gid))
	// 		{
	// 			mdd_tp__add_mbrole(tp, f_mr);
	// 			goto jump_a;
	// 		}

	// 		if (ctx_mr->dim_role == NULL && f_mr->dim_role == NULL)
	// 		{
	// 			mdd_tp__add_mbrole(tp, f_mr);
	// 			goto jump_a;
	// 		}
	// 	}
	// 	mdd_tp__add_mbrole(tp, ctx_mr);
	// jump_a:
	// 	i = i;
	// }

	// for (j = 0; j < frag_sz; j++)
	// {
	// 	MddMemberRole *f_mr = (MddMemberRole *)als_get(tuple_frag->mr_ls, j);
	// 	for (i = 0; i < ctx_sz; i++)
	// 	{
	// 		MddMemberRole *ctx_mr = (MddMemberRole *)als_get(ctx_tuple->mr_ls, i);

	// 		if ((ctx_mr->dim_role != NULL && f_mr->dim_role != NULL) && (ctx_mr->dim_role->gid == f_mr->dim_role->gid))
	// 		{
	// 			goto jump_b;
	// 		}

	// 		if (ctx_mr->dim_role == NULL && f_mr->dim_role == NULL)
	// 		{
	// 			goto jump_b;
	// 		}
	// 	}
	// 	mdd_tp__add_mbrole(tp, f_mr);
	// jump_b:
	// 	j = j;
	// }

	return tp;
}

static MddAxis *ax_def__build(MDContext *md_ctx, AxisDef *ax_def, MddTuple *ctx_tuple, Cube *cube)
{
	MddSet *_set = ids_setdef__build(md_ctx, ax_def->set_def, ctx_tuple, cube);
	MddAxis *ax = mdd_ax__create();
	ax->set = _set;
	ax->posi = ax_def->posi;
	return ax;
}

static unsigned int mdd_ax__len(MddAxis *ax)
{
	return mdd_set__len(ax->set);
}

static unsigned int mdd_set__len(MddSet *set)
{
	return als_size(set->tuples);
}

int mdd_mbr__is_leaf(Member *m)
{
	return (m->bin_attr & MDD_MEMBER__BIN_ATTR_FLAG__NON_LEAF) == 0 ? 1 : 0;
}

void mdd_mbr__set_as_leaf(Member *m)
{
	m->bin_attr = m->bin_attr | MDD_MEMBER__BIN_ATTR_FLAG__NON_LEAF;
}

MddTuple *mdd_tp__create()
{
	MddTuple *tp = mam_alloc(sizeof(MddTuple), OBJ_TYPE__MddTuple, NULL, 0);
	tp->mr_ls = als_new(32, "MddMemberRole *", THREAD_MAM, NULL);
	return tp;
}

int Tuple__cmp(MddTuple *t1, MddTuple *t2)
{
	int i, sz = als_size(t1->mr_ls);
	if (sz != als_size(t2->mr_ls))
		return 1;
	for (i = 0; i < sz; i++)
	{
		MddMemberRole *mr1 = als_get(t1->mr_ls, i);
		MddMemberRole *mr2 = als_get(t2->mr_ls, i);
		if (MemberRole__cmp(mr1, mr2) != 0)
			return 1;
	}
	return 0;
}

MddMemberRole *mdd_mr__create(Member *m, DimensionRole *dr)
{
	MddMemberRole *mr = mam_alloc(sizeof(MddMemberRole), OBJ_TYPE__MddMemberRole, NULL, 0);
	mr->member = m;
	mr->dim_role = dr;
	return mr;
}

int MemberRole__cmp(MddMemberRole *mr_1, MddMemberRole *mr_2)
{

	if (mr_1->member == NULL || mr_2->member == NULL)
		return 1;

	if (mr_1->dim_role && mr_2->dim_role)
	{
		return mr_1->member->gid == mr_2->member->gid && mr_1->dim_role->gid == mr_2->dim_role->gid ? 0 : 1;
	}

	if (mr_1->dim_role == NULL && mr_2->dim_role == NULL)
	{
		return mr_1->member->gid == mr_2->member->gid ? 0 : 1;
	}

	return 1;
}

MddTuple *ids_setdef__head_ref_tuple(MDContext *md_ctx, SetDef *set_def, MddTuple *context_tuple, Cube *cube)
{
	if (set_def->t_cons == SET_DEF__TUP_DEF_LS)
	{
		void *obj = als_get(set_def->tuple_def_ls, 0);
		switch (obj_type_of(obj))
		{
		case OBJ_TYPE__TupleDef:
			return ids_tupledef__build(md_ctx, obj, context_tuple, cube);
		case OBJ_TYPE__GeneralChainExpression:
			MddTuple *tuple = gce_transform(md_ctx, obj, context_tuple, cube);
			assert(obj_type_of(tuple) == OBJ_TYPE__MddTuple);
			return tuple;
		default:
			log_print("[ error ] exit. Program logic error.\n");
			exit(EXIT_FAILURE);
		}
	}
	else if (set_def->t_cons == SET_DEF__SET_FUNCTION)
	{
		if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnChildren)
		{
			MddSet *set = SetFnChildren_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnMembers)
		{
			MddSet *set = SetFnMembers_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnFilter)
		{
			MddSet *set = SetFnFilter_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnCrossJoin)
		{
			MddSet *set = SetFnCrossJoin_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnLateralMembers)
		{
			MddSet *set = SetFnLateralMembers_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnOrder)
		{
			MddSet *set = SetFnOrder_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnTopCount)
		{
			MddSet *set = SetFnTopCount_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnExcept)
		{
			MddSet *set = SetFnExcept_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnYTD)
		{
			SetFnYTD *ytd = set_def->set_fn;
			if (ytd->mbr_def == NULL)
				return NULL;
			MddSet *set = SetFnYTD_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnDescendants)
		{
			MddSet *set = SetFnDescendants_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnTail)
		{
			MddSet *set = SetFnTail_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnBottomOrTopPercent)
		{
			MddSet *set = SetFnBottomOrTopPercent_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnUnion)
		{
			MddSet *set = SetFnUnion_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnIntersect)
		{
			MddSet *set = SetFnIntersect_evolving(md_ctx, set_def->set_fn, cube, context_tuple);
			return als_get(set->tuples, 0);
		}
		else
		{
			log_print("[ error ] - ids_setdef__head_ref_tuple() obj_type_of(set_def->set_fn) = %d\n", obj_type_of(set_def->set_fn));
			exit(1);
		}
	}
	else if (set_def->t_cons == SET_DEF__VAR_OR_BLOCK)
	{
		int i, sz = als_size(md_ctx->select_def->set_formulas);
		for (i = 0; i < sz; i++)
		{
			SetFormula *sf = als_get(md_ctx->select_def->set_formulas, i);
			if (strcmp(set_def->var_block, sf->var_block) == 0)
			{
				return ids_setdef__head_ref_tuple(md_ctx, sf->set_def, context_tuple, cube);
			}
		}
	}
	else if (set_def->t_cons == SET_DEF__MDE_UNI_PATH) {

		// Handling special cases.
		// When a set definition cannot give a reference tuple for influencing the context, it simply return a NULL.
		if (als_size(set_def->up->list) == 1) {
			void *obj = als_get(set_def->up->list, 0);
			short _type;
			enum_oms _strat;
			MemAllocMng *_mam;
			obj_info(obj, &_type, &_strat, &_mam);
			if (_type == OBJ_TYPE__SetFnYTD && ((SetFnYTD *)obj)->mbr_def == NULL) {
				return NULL;
			}
		}

		void *var = up_evolving(md_ctx, set_def->up, cube, context_tuple);

		short obj_type;
		enum_oms obj_strat;
		MemAllocMng *mam;
		obj_info(var, &obj_type, &obj_strat, &mam);

		if (obj_type != OBJ_TYPE__MddSet) {
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "Exception: A Set object is needed here.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		MddSet *set = var;
		return als_get(set->tuples, 0);
	}
	else
	{
		log_print("[ error ] - ids_setdef__head_ref_tuple() set_def->t_cons = %d\n", set_def->t_cons);
		exit(1);
	}
}

static MddTuple *ids_tupledef__build(MDContext *md_ctx, TupleDef *t_def, MddTuple *context_tuple, Cube *cube)
{
	MddTuple *t = (MddTuple *)mdd_tp__create();
	int i, len = als_size(t_def->ms_def->mbr_def_ls);
	for (i = 0; i < len; i++)
	{
		MemberDef *m_def = als_get(t_def->ms_def->mbr_def_ls, i);
		MddMemberRole *mr = ids_mbrsdef__build(md_ctx, m_def, context_tuple, cube);
		mdd_tp__add_mbrole(t, mr);
	}
	return t;
}

MddMemberRole *ids_mbrsdef__build(MDContext *md_ctx, MemberDef *m_def, MddTuple *context_tuple, Cube *cube)
{
	MddMemberRole *member_role_ = NULL;

	if (m_def->t_cons == MEMBER_DEF__MBR_ABS_PATH)
	{
		char *dim_role_name = als_get(m_def->mbr_abs_path, 0);
		DimensionRole *dr = cube__dim_role(cube, dim_role_name);
		Dimension *dim;

		if (dr)
		{ // dim is not measure dimension
			int i, dims_pool_size = als_size(dims_pool);
			for (i = 0; i < dims_pool_size; i++)
			{
				dim = (Dimension *)als_get(dims_pool, i);
				if (dim->gid == dr->dim_gid)
					break;
			}

			ArrayList *mbr_path = als_new(32, "char *", THREAD_MAM, NULL);
			int ap_len = als_size(m_def->mbr_abs_path);
			for (i = 1; i < ap_len; i++)
			{
				als_add(mbr_path, als_get(m_def->mbr_abs_path, i));
			}

			Member *mbr = dim__find_mbr(dim, mbr_path);

			if (mbr)
			{ // entity dimension member
				member_role_ = mdd_mr__create(mbr, dr);
				return member_role_;
			}
			else if (md_ctx->select_def->member_formulas)
			{ // formula member
				int f_sz = als_size(md_ctx->select_def->member_formulas);
				for (i = 0; i < f_sz; i++)
				{
					MemberFormula *f = als_get(md_ctx->select_def->member_formulas, i);
					if ((strcmp(als_get(f->path, 0), als_get(m_def->mbr_abs_path, 0)) == 0) && (strcmp(als_get(f->path, 1), als_get(m_def->mbr_abs_path, 1)) == 0))
					{
						MddMemberRole *mr = mdd_mr__create(NULL, dr);
						mr->member_formula = f;
						member_role_ = mr;
						return member_role_;
					}
				}
				MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
				thrd_mam->exception_desc = "exception: Unrecognized dimension member.";
				longjmp(thrd_mam->excep_ctx_env, -1);
			}
			else
			{
				MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
				thrd_mam->exception_desc = "exception: Unrecognized dimension member.";
				longjmp(thrd_mam->excep_ctx_env, -1);
			}
		}
		else
		{
			if (strcmp(dim_role_name, STANDARD_MEASURE_DIMENSION) == 0)
			{
				// measure dimension
				char *mea_m_name = als_get(m_def->mbr_abs_path, 1);
				int i, mea_m_count = als_size(cube->measure_mbrs);
				for (i = 0; i < mea_m_count; i++)
				{
					Member *mbr = (Member *)als_get(cube->measure_mbrs, i);
					if (strcmp(mbr->name, mea_m_name) == 0)
					{
						member_role_ = mdd_mr__create(mbr, dr);
						return member_role_;
					}
				}
			}

			if (md_ctx == NULL || md_ctx->select_def->member_formulas == NULL)
				goto unknown_dim_role_exception;

			// formula member
			int f_sz = als_size(md_ctx->select_def->member_formulas);
			for (int i = 0; i < f_sz; i++)
			{
				MemberFormula *f = als_get(md_ctx->select_def->member_formulas, i);
				if ((strcmp(als_get(f->path, 0), als_get(m_def->mbr_abs_path, 0)) == 0) && (strcmp(als_get(f->path, 1), als_get(m_def->mbr_abs_path, 1)) == 0))
				{
					MddMemberRole *mr = mdd_mr__create(NULL, NULL);
					mr->member_formula = f;
					member_role_ = mr;
					return member_role_;
				}
			}

		unknown_dim_role_exception:
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: (0) An undefined dimension role was encountered.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}
	else if (m_def->t_cons == MEMBER_DEF__MBR_FUNCTION)
	{
		if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberFnParent)
		{
			member_role_ = MemberFnParent_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberFnCurrentMember)
		{
			member_role_ = MemberFnCurrentMember_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberFnPrevMember)
		{
			member_role_ = MemberFnPrevMember_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberRoleFnParallelPeriod)
		{
			member_role_ = MemberRoleFnParallelPeriod_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberRoleFnClosingPeriod)
		{
			member_role_ = MemberRoleFnClosingPeriod_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberRoleFnOpeningPeriod)
		{
			member_role_ = MemberRoleFnOpeningPeriod_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberRoleFnFirstChild)
		{
			member_role_ = MemberRoleFnFirstChild_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberRoleFnLastChild)
		{
			member_role_ = MemberRoleFnLastChild_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberRoleFnFirstSibling)
		{
			member_role_ = MemberRoleFnFirstSibling_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberRoleFnLastSibling)
		{
			member_role_ = MemberRoleFnLastSibling_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else if (obj_type_of(m_def->member_fn) == OBJ_TYPE__MemberRoleFnLag)
		{
			member_role_ = MemberRoleFnLag_evolving(md_ctx, m_def->member_fn, context_tuple, cube);
			return member_role_;
		}
		else
		{
			log_print("[ error ] - ids_mbrsdef__build() obj_type_of(m_def->member_fn)\n");
			exit(1);
		}
	}
	else
	{
		log_print("[error] Unknown type about defining dimension member.\n");
		exit(1);
	}
	log_print("[ error ] Incorrect program execution path, causing the program to exit.\n");
	exit(1);
}

void mdd_tp__add_mbrole(MddTuple *t, MddMemberRole *mr)
{
	als_add(t->mr_ls, mr);
}

DimensionRole *cube__dim_role(Cube *cube, char *dim_role_name)
{
	int i, dr_ls_zs = als_size(cube->dim_role_ls);
	for (i = 0; i < dr_ls_zs; i++)
	{
		DimensionRole *dr = (DimensionRole *)als_get(cube->dim_role_ls, i);
		if (strcmp(dr->name, dim_role_name) == 0)
			return dr;
	}
	// log_print("[warn] no DimensionRole that name is [%s]\n", dim_role_name);
	return NULL;
}

Member *dim__find_mbr(Dimension *dim, ArrayList *mbr_name_path)
{
	int i, len = als_size(mbr_name_path);
	Member *m = find_member_lv1(dim, als_get(mbr_name_path, 0));
	for (i = 1; i < len; i++)
	{
		if (!m)
			break;
		m = find_member_child(m, als_get(mbr_name_path, i));
	}

	return m;
}

MddSet *mdd_set__create()
{
	MddSet *set = mam_alloc(sizeof(MddSet), OBJ_TYPE__MddSet, NULL, 0);
	set->tuples = als_new(64, "MddTuple *", THREAD_MAM, NULL);
	return set;
}

unsigned int mdd_set__max_tuple_len(MddSet *set)
{
	unsigned int max = als_size(((MddTuple *)als_get(set->tuples, 0))->mr_ls);
	for (int i = 1; i < als_size(set->tuples); i++)
	{
		if (als_size(((MddTuple *)als_get(set->tuples, i))->mr_ls) > max)
			max = als_size(((MddTuple *)als_get(set->tuples, i))->mr_ls);
	}
	return max;
}

MddAxis *mdd_ax__create()
{
	return mam_alloc(sizeof(MddAxis), OBJ_TYPE__MddAxis, NULL, 0);
}

MddSet *ids_setdef__build(MDContext *md_ctx, SetDef *set_def, MddTuple *ctx_tuple, Cube *cube)
{
	if (set_def->t_cons == SET_DEF__TUP_DEF_LS)
	{
		MddSet *set = mdd_set__create();
		int i, sz = als_size(set_def->tuple_def_ls);
		for (i = 0; i < sz; i++)
		{
			void *obj = als_get(set_def->tuple_def_ls, i);
			if (obj_type_of(obj) == OBJ_TYPE__TupleDef)
			{
				mddset__add_tuple(set, ids_tupledef__build(md_ctx, obj, ctx_tuple, cube));
				continue;
			}
			assert(obj_type_of(obj) == OBJ_TYPE__GeneralChainExpression);
			MddTuple *tp = gce_transform(md_ctx, obj, ctx_tuple, cube);
			assert(obj_type_of(tp) == OBJ_TYPE__MddTuple);
			mddset__add_tuple(set, tp);
		}
		return set;
	}
	else if (set_def->t_cons == SET_DEF__SET_FUNCTION)
	{
		if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnChildren)
		{
			return SetFnChildren_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnMembers)
		{
			return SetFnMembers_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnFilter)
		{
			return SetFnFilter_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnCrossJoin)
		{
			return SetFnCrossJoin_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnLateralMembers)
		{
			return SetFnLateralMembers_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnOrder)
		{
			return SetFnOrder_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnTopCount)
		{
			return SetFnTopCount_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnExcept)
		{
			return SetFnExcept_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnYTD)
		{
			return SetFnYTD_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnDescendants)
		{
			return SetFnDescendants_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnTail)
		{
			return SetFnTail_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnBottomOrTopPercent)
		{
			return SetFnBottomOrTopPercent_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnUnion)
		{
			return SetFnUnion_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SetFnIntersect)
		{
			return SetFnIntersect_evolving(md_ctx, set_def->set_fn, cube, ctx_tuple);
		}
		else
		{
			log_print("[warn] - ids_setdef__build() obj_type_of(set_def->set_fn) = %d\n", obj_type_of(set_def->set_fn));
			exit(1);
		}
	}
	else if (set_def->t_cons == SET_DEF__VAR_OR_BLOCK)
	{
		int i, sz = als_size(md_ctx->select_def->set_formulas);
		for (i = 0; i < sz; i++)
		{
			SetFormula *sf = als_get(md_ctx->select_def->set_formulas, i);
			if (strcmp(set_def->var_block, sf->var_block) == 0)
			{
				return ids_setdef__build(md_ctx, sf->set_def, ctx_tuple, cube);
			}
		}
	}
	else if (set_def->t_cons == SET_DEF__MDE_UNI_PATH)
	{
		void *entity = up_evolving(md_ctx, set_def->up, cube, ctx_tuple);

		short obj_type;
		enum_oms obj_strat;
		MemAllocMng *mam;
		obj_info(entity, &obj_type, &obj_strat, &mam);

		if (obj_type == OBJ_TYPE__MddSet) {
			return (MddSet *)entity;
		} else if (obj_type == OBJ_TYPE__MddTuple) {
			log_print("[ warn ] Undeveloped functionality!\n");
			exit(EXIT_FAILURE);
		} else if (obj_type == OBJ_TYPE__MddMemberRole) {
			log_print("[ warn ] Undeveloped functionality!\n");
			exit(EXIT_FAILURE);
		} else {
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "Exception: 0 - A Set object is needed here.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}
	else
	{
		log_print("[warn] wrong SetDef::t_cons\n");
		exit(1);
	}
}

void mddset__add_tuple(MddSet *s, MddTuple *t)
{
	als_add(s->tuples, t);
}

MddTuple *mdd_ax__get_tuple(MddAxis *ax, int idx)
{
	return als_get(ax->set->tuples, idx);
}

MddTuple *_MddTuple__mergeTuples(MddTuple **tps, int count)
{
	if (count < 2)
		return tps[0];
	MddTuple *tuple = tuple__merge(tps[0], tps[1]);
	int i;
	for (i = 2; i < count; i++)
		tuple = tuple__merge(tuple, tps[i]);
	return tuple;
}

ArrayList *Cube_find_date_dim_roles(Cube *cube)
{
	ArrayList *date_drs = als_new(8, "DimensionRole *", THREAD_MAM, NULL);
	int i, dr_count = als_size(cube->dim_role_ls);
	for (i = 0; i < dr_count; i++)
	{
		DimensionRole *dr = als_get(cube->dim_role_ls, i);
		if (strcmp(find_dim_by_gid(dr->dim_gid)->name, "Calendar") == 0 || strcmp(find_dim_by_gid(dr->dim_gid)->name, "Date") == 0)
			als_add(date_drs, dr);
	}
	return date_drs;
}

void Cube_print(Cube *c)
{
	log_print(">>> [ Cube info ] @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ addr < %p >\n", c);
	log_print("\t     name - %s\n", c->name);
	log_print("\t      gid - %lu\n", c->gid);
}

void Tuple_print(MddTuple *tuple)
{
	log_print("{\n");
	log_print("\"type\": \"Tuple\",\n");
	log_print("\"mr_ls\": [\n");

	unsigned int i, len = als_size(tuple->mr_ls);
	for (i = 0; i < len; i++)
	{
		MemberRole_print(als_get(tuple->mr_ls, i));
		if (i < len - 1)
			log_print(",\n");
	}

	log_print("]\n");
	log_print("}\n");
}

int MddMemberRole_cmp(void *mr, void *oth)
{
	MddMemberRole *mr_a = (MddMemberRole *)mr;
	MddMemberRole *mr_o = (MddMemberRole *)oth;
	if (mr_a->dim_role && mr_o->dim_role)
	{
		return mr_o->dim_role->sn - mr_a->dim_role->sn;
	}
	else if (mr_a->dim_role && !mr_o->dim_role)
	{
		return 1;
	}
	else if (mr_o->dim_role && !mr_a->dim_role)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void MemberRole_print(MddMemberRole *mr)
{
	log_print("{\n");
	log_print("\"type\": \"MemberRole\",\n");
	if (mr->member)
	{
		log_print("\"member\": ");
		Member_print(mr->member);
	}
	else
	{
		log_print("\"member_formula\": ");
		MemberFormula_print(mr->member_formula);
	}
	log_print(",\n");
	log_print("\"dim_role\": ");
	if (mr->dim_role)
	{
		DimensionRole_print(mr->dim_role);
	}
	else
	{
		log_print("\"*** measure dimension role ***\"");
	}
	log_print("}\n");
}

void Member_print(Member *m)
{
	log_print("{ \"type\": \"Member\", \"name\": \"%s\" }\n", m->name);
}

void DimensionRole_print(DimensionRole *dr)
{
	if (dr)
		log_print("{ \"DimRole\": \"%s\", \"sn\": %d }\n", dr->name, dr->sn);
	else
		log_print("null\n");
}

void mdd__gen_mbr_abs_path(Member *m)
{
	if (m->abs_path)
		return;

	m->abs_path = mam_alloc(m->lv * sizeof(md_gid), OBJ_TYPE__RAW_BYTES, meta_mam, 0);

	Member *current_m = m;

	int i;
	for (i = m->lv - 1; i >= 0; i--)
	{
		m->abs_path[i] = current_m->gid;
		if (current_m->p_gid)
			current_m = find_member_by_gid(current_m->p_gid);
	}
}

Member *find_member_by_gid(md_gid m_gid)
{
	int i;
	for (i = 0; i < als_size(member_pool); i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->gid == m_gid)
			return m;
	}
	return NULL;
}

void Expression_evaluate(MDContext *md_ctx, Expression *exp, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{
	grid_data->val = 0;
	grid_data->null_flag = 1;

	int p_sz = als_size(exp->plus_terms);
	int m_sz = als_size(exp->minus_terms);
	int i;

	GridData tmp;

	for (i = 0; i < p_sz; i++)
	{
		Term *term = als_get(exp->plus_terms, i);
		// val += Term_evaluate(md_ctx, term, cube, ctx_tuple, &tmp);
		Term_evaluate(md_ctx, term, cube, ctx_tuple, &tmp);
		if (tmp.null_flag == 0)
		{
			grid_data->val += tmp.val;
			grid_data->null_flag = 0;
		}
		else
		{
			grid_data->null_flag = 1;
			return;
		}
	}

	for (i = 0; i < m_sz; i++)
	{
		Term *term = als_get(exp->minus_terms, i);
		// val -= Term_evaluate(md_ctx, term, cube, ctx_tuple, &tmp);
		Term_evaluate(md_ctx, term, cube, ctx_tuple, &tmp);
		if (tmp.null_flag == 0)
		{
			grid_data->val -= tmp.val;
			grid_data->null_flag = 0;
		}
		else
		{
			grid_data->null_flag = 1;
			return;
		}
	}
}

void Term_evaluate(MDContext *md_ctx, Term *term, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{
	grid_data->val = 1;
	grid_data->null_flag = 1;

	int m_sz = als_size(term->mul_factories);
	int d_sz = als_size(term->div_factories);
	int i;

	GridData tmp;

	for (i = 0; i < m_sz; i++)
	{
		Factory *fac = als_get(term->mul_factories, i);
		Factory_evaluate(md_ctx, fac, cube, ctx_tuple, &tmp);
		if (tmp.null_flag == 0)
		{
			grid_data->val *= tmp.val;
			grid_data->null_flag = 0;
		}
		else
		{
			grid_data->null_flag = 1;
			return;
		}
	}

	for (i = 0; i < d_sz; i++)
	{
		Factory *fac = als_get(term->div_factories, i);
		Factory_evaluate(md_ctx, fac, cube, ctx_tuple, &tmp);
		if (tmp.null_flag == 0)
		{
			grid_data->val /= tmp.val;
			grid_data->null_flag = 0;
		}
		else
		{
			grid_data->null_flag = 1;
			return;
		}
	}
}

void Factory_evaluate(MDContext *md_ctx, Factory *fac, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{
	if (fac->t_cons == FACTORY_DEF__TUP_DEF)
	{
		MddTuple *tuple = ids_tupledef__build(md_ctx, fac->tuple_def, ctx_tuple, cube);
		tuple = tuple__merge(ctx_tuple, tuple);
		do_calculate_measure_value(md_ctx, cube, tuple, grid_data);
		return;
	}
	else if (fac->t_cons == FACTORY_DEF__DECIMAL)
	{
		grid_data->val = fac->decimal;
		grid_data->null_flag = 0;
		return;
	}
	else if (fac->t_cons == FACTORY_DEF__EXPRESSION)
	{
		Expression_evaluate(md_ctx, (Expression *)(fac->exp), cube, ctx_tuple, grid_data);
		return;
	}
	else if (fac->t_cons == FACTORY_DEF__EXP_FN)
	{
		if (obj_type_of(fac->exp) == OBJ_TYPE__ExpFnSum)
		{
			ExpFnSum_evolving(md_ctx, fac->exp, cube, ctx_tuple, grid_data);
			return;
		}
		else if (obj_type_of(fac->exp) == OBJ_TYPE__ExpFnCount)
		{
			ExpFnCount_evolving(md_ctx, fac->exp, cube, ctx_tuple, grid_data);
			return;
		}
		else if (obj_type_of(fac->exp) == OBJ_TYPE__ExpFnLookUpCube)
		{
			ExpFnLookUpCube_evolving(md_ctx, fac->exp, cube, ctx_tuple, grid_data);
			return;
		}
		else if (obj_type_of(fac->exp) == OBJ_TYPE__ExpFnIif)
		{
			ExpFnIif_evolving(md_ctx, fac->exp, cube, ctx_tuple, grid_data);
			return;
		}
		else if (obj_type_of(fac->exp) == OBJ_TYPE__ExpFnCoalesceEmpty)
		{
			ExpFnCoalesceEmpty_evolving(md_ctx, fac->exp, cube, ctx_tuple, grid_data);
			return;
		}
		else
		{
			log_print("[ error ] - Factory_evaluate() - Unknown expression function type.\n");
			exit(1);
		}
	}
	else
	{
		log_print("[ error ] - Factory_evaluate() <program exit>\n");
		exit(1);
	}
}

void BooleanExpression_evaluate(MDContext *md_ctx, BooleanExpression *boolExp, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{
	grid_data->null_flag = 0;
	grid_data->type = GRIDDATA_TYPE_BOOL;
	grid_data->boolean = GRIDDATA_BOOL_FALSE;
	int i, len = als_size(boolExp->terms);
	GridData data;
	for (i = 0; i < len; i++)
	{
		BooleanTerm *term = als_get(boolExp->terms, i);
		BooleanTerm_evaluate(md_ctx, term, cube, ctx_tuple, &data);
		if (data.boolean == GRIDDATA_BOOL_TRUE)
		{
			grid_data->boolean = GRIDDATA_BOOL_TRUE;
			return;
		}
	}
}

void BooleanTerm_evaluate(MDContext *md_ctx, BooleanTerm *boolTerm, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{
	grid_data->null_flag = 0;
	grid_data->type = GRIDDATA_TYPE_BOOL;
	grid_data->boolean = GRIDDATA_BOOL_TRUE;
	int i, len = als_size(boolTerm->factories);
	GridData data;
	for (i = 0; i < len; i++)
	{
		BooleanFactory *fac = als_get(boolTerm->factories, i);
		BooleanFactory_evaluate(md_ctx, fac, cube, ctx_tuple, &data);
		if (data.boolean == GRIDDATA_BOOL_FALSE)
		{
			grid_data->boolean = GRIDDATA_BOOL_FALSE;
			return;
		}
	}
}

void BooleanFactory_evaluate(MDContext *md_ctx, BooleanFactory *boolFac, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{
	grid_data->null_flag = 0;
	grid_data->type = GRIDDATA_TYPE_BOOL;
	grid_data->boolean = GRIDDATA_BOOL_FALSE;
	if (boolFac->boolean_expression)
	{
		BooleanExpression_evaluate(md_ctx, boolFac->boolean_expression, cube, ctx_tuple, grid_data);
		return;
	}
	GridData left__val;
	GridData right_val;
	Expression_evaluate(md_ctx, boolFac->left__exp, cube, ctx_tuple, &left__val);
	Expression_evaluate(md_ctx, boolFac->right_exp, cube, ctx_tuple, &right_val);

	if (left__val.null_flag != 0 || right_val.null_flag != 0)
		return;

	if (((boolFac->op == BOOL_FAC_OPS__LESS) && (left__val.val < right_val.val)) || ((boolFac->op == BOOL_FAC_OPS__LESS_EQ) && (left__val.val <= right_val.val)) || ((boolFac->op == BOOL_FAC_OPS__EQ) && (left__val.val == right_val.val)) || ((boolFac->op == BOOL_FAC_OPS__NOT_EQ) && (left__val.val != right_val.val)) || ((boolFac->op == BOOL_FAC_OPS__GREA) && (left__val.val > right_val.val)) || ((boolFac->op == BOOL_FAC_OPS__GREA_EQ) && (left__val.val >= right_val.val)))
	{
		grid_data->boolean = GRIDDATA_BOOL_TRUE;
	}
}

MddSet *SetFnChildren_evolving(MDContext *md_ctx, void *fn, Cube *cube, MddTuple *ctx_tuple)
{
	MddMemberRole *parent_mr = ids_mbrsdef__build(md_ctx, ((SetFnChildren *)fn)->m_def, ctx_tuple, cube);
	Member *parent = parent_mr->member;
	MddSet *set = mdd_set__create();
	int i, sz = als_size(member_pool);
	for (i = 0; i < sz; i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->p_gid == parent->gid)
		{
			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(m, parent_mr->dim_role));
			mddset__add_tuple(set, tuple);
		}
	}
	return set;
}

MddSet *SetFnMembers_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{
	SetFnMembers *fn = (SetFnMembers *)set_fn;
	DimensionRole *dr = NULL;
	int i, cube_dr_count = als_size(cube->dim_role_ls);
	for (i = 0; i < cube_dr_count; i++)
	{
		dr = als_get(cube->dim_role_ls, i);
		if (strcmp(dr->name, fn->dr_def->name) == 0)
			break;
		dr = NULL;
	}

	MddSet *set = mdd_set__create();

	if (dr == NULL)
	{
		if (strcmp(STANDARD_MEASURE_DIMENSION, fn->dr_def->name))
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: (1) An undefined dimension role was encountered.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		// measure dimension role
		int mm_sz = als_size(cube->measure_mbrs);
		for (i = 0; i < mm_sz; i++)
		{
			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(als_get(cube->measure_mbrs, i), NULL));
			mddset__add_tuple(set, tuple);
		}
		return set;
	}

	int mpool_sz = als_size(member_pool);
	for (i = 0; i < mpool_sz; i++)
	{
		Member *member = als_get(member_pool, i);
		if (member->dim_gid != dr->dim_gid)
			continue;
		MddTuple *tuple = mdd_tp__create();

		MddMemberRole *mr = NULL;

		if (!strcmp(fn->option, "LEAFS"))
		{
			if (member->bin_attr & 1) // not leaf
				continue;
			goto gtf;
		}

		if (!strcmp(fn->option, "NOT_LEAFS"))
		{
			if ((member->bin_attr & 1) != 1) // it is leaf
				continue;
			goto gtf;
		}

	gtf:
		mr = mdd_mr__create(member, dr);
		mdd_tp__add_mbrole(tuple, mr);
		mddset__add_tuple(set, tuple);
	}
	return set;
}

MddSet *SetFnCrossJoin_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{

	SetFnCrossJoin *crossJoin = set_fn;
	MddSet *set_ctx = ids_setdef__build(md_ctx, als_get(crossJoin->set_def_ls, 0), ctx_tuple, cube);
	ArrayList *ctx_tuple_ls = set_ctx->tuples;
	int i, ls_len = als_size(crossJoin->set_def_ls);
	for (i = 1; i < ls_len; i++)
	{
		MddSet *set = ids_setdef__build(md_ctx, als_get(crossJoin->set_def_ls, i), ctx_tuple, cube);
		ArrayList *tuple_ls = als_new(512, "MddTuple *", THREAD_MAM, NULL);
		int j, k, ctx_sz = als_size(ctx_tuple_ls), set_sz = als_size(set->tuples);

		for (j = 0; j < ctx_sz; j++)
		{
			for (k = 0; k < set_sz; k++)
			{
				MddTuple *ctx_tuple = als_get(ctx_tuple_ls, j);
				MddTuple *tuple_frag = als_get(set->tuples, k);
				MddTuple *merged_tuple = tuple__merge(ctx_tuple, tuple_frag);
				als_add(tuple_ls, merged_tuple);
			}
		}

		ctx_tuple_ls = tuple_ls;
	}

	MddSet *join_set = mdd_set__create();

	for (i = 0; i < als_size(ctx_tuple_ls); i++)
	{
		mddset__add_tuple(join_set, als_get(ctx_tuple_ls, i));
	}

	return join_set;
}

MddSet *SetFnFilter_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{
	MddSet *result = mdd_set__create();
	SetFnFilter *filter = set_fn;
	MddSet *set = ids_setdef__build(md_ctx, filter->set_def, ctx_tuple, cube);
	GridData data;
	int i, len = als_size(set->tuples);
	for (i = 0; i < len; i++)
	{
		MddTuple *tuple = als_get(set->tuples, i);
		BooleanExpression_evaluate(md_ctx, filter->boolExp, cube, tuple__merge(ctx_tuple, tuple), &data);
		if (data.boolean == GRIDDATA_BOOL_TRUE)
			mddset__add_tuple(result, tuple);
	}
	return result;
}

MddSet *SetFnLateralMembers_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{
	SetFnLateralMembers *latmbrs = (SetFnLateralMembers *)set_fn;
	MddMemberRole *mr = ids_mbrsdef__build(md_ctx, latmbrs->mr_def, ctx_tuple, cube);
	MddSet *set = mdd_set__create();
	int i, sz;

	if (mr->member->dim_gid == cube->measure_dim->gid)
	{
		sz = als_size(cube->measure_mbrs);
		for (i = 0; i < sz; i++)
		{
			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(als_get(cube->measure_mbrs, i), mr->dim_role));
			mddset__add_tuple(set, tuple);
		}
		return set;
	}

	sz = als_size(member_pool);
	for (i = 0; i < sz; i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->dim_gid == mr->member->dim_gid && m->lv == mr->member->lv)
		{
			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(m, mr->dim_role));
			mddset__add_tuple(set, tuple);
		}
	}
	return set;
}

MddSet *SetFnOrder_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{

	SetFnOrder *order = set_fn;

	MddSet *set = ids_setdef__build(md_ctx, order->set, ctx_tuple, cube);
	int i, j, sz = als_size(set->tuples);

	ArrayList *val_ls = als_new(als_size(set->tuples), "double", THREAD_MAM, NULL);
	for (i = 0; i < sz; i++)
	{
		MddTuple *tuple = als_get(set->tuples, i);
		tuple = tuple__merge(ctx_tuple, tuple);
		GridData data;
		Expression_evaluate(md_ctx, order->exp, cube, tuple, &data);
		// log_print("value = % 32lf, null_flag < %d >\n", data.val, data.null_flag);
		als_add(val_ls, *((void **)&(data.val)));
	}

	// Insertion Sort Algorithm
	for (i = 1; i < sz; i++)
	{
		for (j = i; j > 0; j--)
		{
			void *va = als_get(val_ls, j - 1);
			void *vb = als_get(val_ls, j);
			double val_a = *((double *)&va);
			double val_b = *((double *)&vb);

			if (order->option == SET_FN__ORDER_ASC || order->option == SET_FN__ORDER_BASC)
			{
				if (val_b < val_a)
					goto transpose;
				continue;
			}
			else
			{
				// order->option == SET_FN__ORDER_DESC || order->option == SET_FN__ORDER_BDESC
				if (val_b > val_a)
					goto transpose;
				continue;
			}

		transpose:
			ArrayList_set(val_ls, j - 1, vb);
			ArrayList_set(val_ls, j, va);

			MddTuple *tmptp = als_get(set->tuples, j - 1);
			ArrayList_set(set->tuples, j - 1, als_get(set->tuples, j));
			ArrayList_set(set->tuples, j, tmptp);
		}
	}

	return set;
}

MddSet *SetFnTopCount_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{
	SetFnTopCount *top_count = set_fn;

	MddSet *set = ids_setdef__build(md_ctx, top_count->set, ctx_tuple, cube);
	GridData data;
	Expression_evaluate(md_ctx, top_count->count_exp, cube, ctx_tuple, &data);
	int count = data.val;

	int i, j, sz = als_size(set->tuples);

	if (top_count->num_exp)
	{
		ArrayList *val_ls = als_new(als_size(set->tuples), "double", THREAD_MAM, NULL);
		for (i = 0; i < sz; i++)
		{
			MddTuple *tuple = als_get(set->tuples, i);
			tuple = tuple__merge(ctx_tuple, tuple);
			GridData data;
			Expression_evaluate(md_ctx, top_count->num_exp, cube, tuple, &data);
			als_add(val_ls, *((void **)&(data.val)));
		}

		for (i = 0; i < sz - 1; i++)
		{
			for (j = i + 1; j < sz; j++)
			{
				void *vi = als_get(val_ls, i);
				void *vj = als_get(val_ls, j);
				void **vi_p = &vi;
				void **vj_p = &vj;
				double val_i = *((double *)vi_p);
				double val_j = *((double *)vj_p);

				if (val_j > val_i)
				{
					ArrayList_set(val_ls, i, vj);
					ArrayList_set(val_ls, j, vi);

					MddTuple *tmptp = als_get(set->tuples, i);
					ArrayList_set(set->tuples, i, als_get(set->tuples, j));
					ArrayList_set(set->tuples, j, tmptp);
				}
			}
		}
	}

	if (count >= als_size(set->tuples))
		return set;

	MddSet *result = mdd_set__create();
	for (i = 0; i < count; i++)
	{
		MddTuple *tuple = als_get(set->tuples, i);
		mddset__add_tuple(result, tuple);
	}

	return result;
}

MddSet *SetFnExcept_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{

	SetFnExcept *except = set_fn;

	MddSet *set_1 = ids_setdef__build(md_ctx, except->set_1, ctx_tuple, cube);
	MddSet *set_2 = ids_setdef__build(md_ctx, except->set_2, ctx_tuple, cube);

	int i, j, s1_sz = als_size(set_1->tuples), s2_sz = als_size(set_2->tuples);
	MddSet *result = mdd_set__create();
	for (i = 0; i < s1_sz; i++)
	{
		MddTuple *tuple_1 = als_get(set_1->tuples, i);
		for (j = 0; j < s2_sz; j++)
		{
			MddTuple *tuple_2 = als_get(set_2->tuples, j);
			if (Tuple__cmp(tuple_1, tuple_2) == 0)
				goto skip;
		}
		mddset__add_tuple(result, tuple_1);
	skip:
		i = i;
	}

	return result;
}

MddSet *SetFnYTD_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{
	SetFnYTD *ytd = set_fn;

	MddMemberRole *mr;
	Dimension *dim;

	if (ytd->mbr_def)
	{
		mr = ids_mbrsdef__build(md_ctx, ytd->mbr_def, ctx_tuple, cube);
		dim = find_dim_by_gid(mr->dim_role->dim_gid);
	}
	else
	{
		int i, sz = als_size(ctx_tuple->mr_ls);
		for (i = 0; i < sz; i++)
		{
			mr = als_get(ctx_tuple->mr_ls, i);
			if (mr->dim_role == NULL)
			{
				mr = NULL;
				continue;
			}

			dim = find_dim_by_gid(mr->dim_role->dim_gid);
			if (strcmp(dim->name, "Calendar") == 0 || strcmp(dim->name, "Date") == 0)
				break;

			mr = NULL;
		}
	}

	Level *year_lv;
	int i, sz = als_size(levels_pool);
	for (i = 0; i < sz; i++)
	{
		year_lv = als_get(levels_pool, i);
		if (year_lv->dim_gid == dim->gid && strcmp(year_lv->name, "year") == 0)
			break;
		year_lv = NULL;
	}

	ArrayList *descendants = mdd__lv_ancestor_peer_descendants(year_lv, mr->member);
	sz = als_size(descendants);
	MddSet *result = mdd_set__create();
	for (i = 0; i < sz; i++)
	{
		MddTuple *tuple = mdd_tp__create();
		mdd_tp__add_mbrole(tuple, mdd_mr__create(als_get(descendants, i), mr->dim_role));
		mddset__add_tuple(result, tuple);
		if (((Member *)als_get(descendants, i))->gid == mr->member->gid)
			break;
	}

	return result;
}

MddSet *SetFnDescendants_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{

	SetFnDescendants *desc = set_fn;

	MddMemberRole *mr = ids_mbrsdef__build(md_ctx, desc->mbr_def, ctx_tuple, cube);

	MddSet *result = mdd_set__create();

	ArrayList *descendants = Member__descendants(mr->member);

	if (desc->lvr_def == NULL && desc->distance == NULL)
	{

		int i, sz = als_size(descendants);
		for (i = 0; i < sz; i++)
		{
			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(als_get(descendants, i), mr->dim_role));
			mddset__add_tuple(result, tuple);
		}
		return result;
	}

	if (desc->lvr_def)
	{
		LevelRole *lr = LevelRoleDef_interpret(md_ctx, desc->lvr_def, ctx_tuple, cube);

		int i, sz = als_size(descendants);
		for (i = 0; i < sz; i++)
		{
			Member *mbr = als_get(descendants, i);
			switch (desc->flag)
			{
			case SET_FN__DESCENDANTS_OPT_SELF:
				if (mbr->lv != lr->lv->level)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_AFTER:
				if (mbr->lv <= lr->lv->level)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_BEFORE:
				if (mbr->lv >= lr->lv->level)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_BEFORE_AND_AFTER:
				if (mbr->lv == lr->lv->level)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_SELF_AND_AFTER:
				if (mbr->lv < lr->lv->level)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_SELF_AND_BEFORE:
				if (mbr->lv > lr->lv->level)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_SELF_BEFORE_AFTER:
				// do nothing
				break;
			case SET_FN__DESCENDANTS_OPT_LEAVES:
				if (mdd_mbr__is_leaf(mbr) == 0 || mbr->lv > lr->lv->level)
					continue;
				break;
			default:
				log_print("[ error ] program exit, cause by: worry value of set function Descendants option flag < %c >\n", desc->flag);
				exit(1);
			}
			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(mbr, mr->dim_role));
			mddset__add_tuple(result, tuple);
		}
		return result;
	}

	if (desc->distance)
	{
		GridData data;
		Expression_evaluate(md_ctx, desc->distance, cube, ctx_tuple, &data);
		int stan_lv = mr->member->lv + data.val;

		int i, sz = als_size(descendants);
		for (i = 0; i < sz; i++)
		{
			Member *mbr = als_get(descendants, i);
			switch (desc->flag)
			{
			case SET_FN__DESCENDANTS_OPT_SELF:
				if (mbr->lv != stan_lv)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_AFTER:
				if (mbr->lv <= stan_lv)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_BEFORE:
				if (mbr->lv >= stan_lv)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_BEFORE_AND_AFTER:
				if (mbr->lv == stan_lv)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_SELF_AND_AFTER:
				if (mbr->lv < stan_lv)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_SELF_AND_BEFORE:
				if (mbr->lv > stan_lv)
					continue;
				break;
			case SET_FN__DESCENDANTS_OPT_SELF_BEFORE_AFTER:
				// do nothing
				break;
			case SET_FN__DESCENDANTS_OPT_LEAVES:
				if (mdd_mbr__is_leaf(mbr) == 0 || mbr->lv > stan_lv)
					continue;
				break;
			default:
				log_print("[ error ] program exit, cause by: worry value of set function Descendants option flag < %c >\n", desc->flag);
				exit(1);
			}
			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(mbr, mr->dim_role));
			mddset__add_tuple(result, tuple);
		}
		return result;
	}
}

MddSet *SetFnTail_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{
	SetFnTail *tail = set_fn;
	MddSet *set = ids_setdef__build(md_ctx, tail->set, ctx_tuple, cube);
	int count = 1;
	if (tail->count)
	{
		GridData data;
		Expression_evaluate(md_ctx, tail->count, cube, ctx_tuple, &data);
		count = data.val;
	}
	if (count >= als_size(set->tuples))
		return set;

	MddSet *result = mdd_set__create();
	int i, sz = als_size(set->tuples);
	for (i = sz - count; i < sz; i++)
	{
		mddset__add_tuple(result, als_get(set->tuples, i));
	}
	return result;
}

MddSet *SetFnBottomOrTopPercent_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{
	SetFnBottomOrTopPercent *per = set_fn;
	MddSet *set = ids_setdef__build(md_ctx, per->set, ctx_tuple, cube);
	GridData data;
	Expression_evaluate(md_ctx, per->percentage, cube, ctx_tuple, &data);
	double global = 0, percent = data.val / 100;
	ArrayList *vals = als_new(128, "double", THREAD_MAM, NULL);
	int i, j, sz = als_size(set->tuples);
	for (i = 0; i < sz; i++)
	{
		Expression_evaluate(md_ctx, per->exp, cube, tuple__merge(ctx_tuple, als_get(set->tuples, i)), &data);
		als_add(vals, *((void **)&data.val));
		global += data.val;
	}

	for (i = 1; i < sz; i++)
	{
		for (j = i; j > 0; j--)
		{
			void *va = als_get(vals, j - 1);
			void *vb = als_get(vals, j);

			double val_a = *((double *)&va);
			double val_b = *((double *)&vb);

			if (per->type == SET_FN__BOTTOM_PERCENT)
			{
				if (val_a <= val_b)
					continue;
			}
			else
			{ // per->type == SET_FN__TOP_PERCENT
				if (val_a >= val_b)
					continue;
			}
			ArrayList_set(vals, j - 1, vb);
			ArrayList_set(vals, j, va);
			MddTuple *tmp = als_get(set->tuples, j - 1);
			ArrayList_set(set->tuples, j - 1, als_get(set->tuples, j));
			ArrayList_set(set->tuples, j, tmp);
		}
	}

	MddSet *result = mdd_set__create();
	if (als_size(set->tuples) < 1)
		return result;

	// if (global < 0 && per->type == SET_FN__TOP_PERCENT) {
	// 	mddset__add_tuple(result, als_get(set->tuples, 0));
	// 	return result;
	// }
	if (global <= 0)
	{
		mddset__add_tuple(result, als_get(set->tuples, 0));
		return result;
	}

	double part = 0;
	for (i = 0; i < sz; i++)
	{
		mddset__add_tuple(result, als_get(set->tuples, i));
		void *vi = als_get(vals, i);
		part += *((double *)&vi);
		if (part >= percent * global)
		{ // part / global >= percent
			return result;
		}
	}

	return result;
}

MddSet *SetFnUnion_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{
	SetFnUnion *uni = set_fn;
	ArrayList *tuples = als_new(64, "MddTuple *", THREAD_MAM, NULL);
	int i, j, len = als_size(uni->set_def_ls);
	for (i = len - 1; i >= 0; i--)
	{
		MddSet *set = ids_setdef__build(md_ctx, als_get(uni->set_def_ls, i), ctx_tuple, cube);
		len = als_size(set->tuples);
		for (j = len - 1; j >= 0; j--)
			als_add(tuples, als_get(set->tuples, j));
	}
	MddSet *result = mdd_set__create();
	if (uni->option == SET_FN__UNION_ALL)
	{
		for (i = als_size(tuples) - 1; i >= 0; i--)
		{
			mddset__add_tuple(result, als_get(tuples, i));
		}
	}
	else
	{
		ArrayList *nonredundant = als_new(64, "MddTuple *", THREAD_MAM, NULL);
		len = als_size(tuples);
		for (i = 0; i < len; i++)
		{
			MddTuple *tuple_i = als_get(tuples, i);
			for (j = i + 1; j < len; j++)
			{
				MddTuple *tuple_j = als_get(tuples, j);
				if (Tuple__cmp(tuple_i, tuple_j) == 0)
					goto skip;
			}
			als_add(nonredundant, tuple_i);
		skip:
			i = i;
		}
		for (i = als_size(nonredundant) - 1; i >= 0; i--)
		{
			mddset__add_tuple(result, als_get(nonredundant, i));
		}
	}
	return result;
}

MddSet *SetFnIntersect_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple)
{
	SetFnIntersect *inter = set_fn;
	MddSet *set_0 = ids_setdef__build(md_ctx, als_get(inter->set_def_ls, 0), ctx_tuple, cube);
	if (als_size(inter->set_def_ls) < 2)
		return set_0;
	MddSet *set_1 = ids_setdef__build(md_ctx, als_get(inter->set_def_ls, 1), ctx_tuple, cube);
	MddSet *result = mdd_set__create();
	int i, j, len_0 = als_size(set_0->tuples), len_1 = als_size(set_1->tuples);
	for (i = 0; i < len_0; i++)
	{
		MddTuple *tuple = als_get(set_0->tuples, i);
		for (j = 0; j < len_1; j++)
		{
			MddTuple *tp = als_get(set_1->tuples, j);
			if (Tuple__cmp(tuple, tp) == 0)
				mddset__add_tuple(result, tuple);
		}
	}
	return result;
}

MddMemberRole *MemberFnParent_evolving(MDContext *md_ctx, MemberFnParent *fn_parent, MddTuple *context_tuple, Cube *cube)
{
	// when current member has no parent, return itself.
	MddMemberRole *child_mr = ids_mbrsdef__build(md_ctx, fn_parent->child_def, context_tuple, cube);
	if (child_mr->member->p_gid)
		return mdd_mr__create(find_member_by_gid(child_mr->member->p_gid), child_mr->dim_role);
	else
		return child_mr;
}

MddMemberRole *MemberFnCurrentMember_evolving(MDContext *md_ctx, MemberFnCurrentMember *cm, MddTuple *context_tuple, Cube *cube)
{
	char *dimRole_name = cm->dr_def->name;
	DimensionRole *dimRole = NULL; // NULL mean that current dimension role is measure
	int i, drs_count, mrs_count;
	if (strcmp("measure", dimRole_name) == 0)
		goto point;

	drs_count = als_size(cube->dim_role_ls);
	for (i = 0; i < drs_count; i++)
	{
		dimRole = als_get(cube->dim_role_ls, i);
		if (strcmp(dimRole_name, dimRole->name) == 0)
			goto point;
		dimRole = NULL;
	}

	log_print("[ error ] - MemberFnCurrentMember do not matching DimensionRole - < %s >\n", dimRole_name);
	return NULL;

point:
	mrs_count = als_size(context_tuple->mr_ls);
	for (i = 0; i < mrs_count; i++)
	{
		MddMemberRole *mbrRole = als_get(context_tuple->mr_ls, i);
		if (dimRole == NULL && mbrRole->dim_role == NULL)
			return mbrRole;
		if (dimRole != NULL && mbrRole->dim_role != NULL && strcmp(dimRole->name, mbrRole->dim_role->name) == 0)
			return mbrRole;
	}

	log_print("[ error ] - MemberFnCurrentMember do not matching DimensionRole - < %s >\n", dimRole_name);
	return NULL;
}

MddMemberRole *MemberFnPrevMember_evolving(MDContext *md_ctx, MemberFnPrevMember *pm, MddTuple *context_tuple, Cube *cube)
{
	MddMemberRole *mr = ids_mbrsdef__build(md_ctx, pm->curr_mr, context_tuple, cube);

	int i, len;
	Member *prev = NULL;

	if (mr->dim_role == NULL)
	{
		// measure
		len = als_size(cube->measure_mbrs);
		if (len < 2)
			return mr;

		for (i = 0; i < len; i++)
		{
			Member *mea_m = als_get(cube->measure_mbrs, i);
			if (mea_m->gid >= mr->member->gid)
				continue;
			if (prev == NULL || mea_m->gid > prev->gid)
				prev = mea_m;
		}
		return prev ? mdd_mr__create(prev, mr->dim_role) : mr;
	}

	len = als_size(member_pool);
	for (i = 0; i < len; i++)
	{
		Member *member = als_get(member_pool, i);
		if ((member->dim_gid != mr->member->dim_gid) || (member->lv != mr->member->lv))
			continue;
		if (member->gid >= mr->member->gid)
			continue;
		if (prev == NULL || member->gid > prev->gid)
			prev = member;
	}
	return prev ? mdd_mr__create(prev, mr->dim_role) : mr;
}

// TODO The function is too bloated and needs to be optimized.
MddMemberRole *MemberRoleFnParallelPeriod_evolving(MDContext *md_ctx, MemberRoleFnParallelPeriod *pp, MddTuple *context_tuple, Cube *cube)
{

	// ParallelPeriod()
	if (pp->lvr_def == NULL)
	{

		ArrayList *roles_of_date_dims = Cube_find_date_dim_roles(cube);
		if (als_size(roles_of_date_dims) != 1)
			return NULL;

		DimensionRole *date_dim_role = als_get(roles_of_date_dims, 0);
		MddMemberRole *date_mr = NULL;

		int i, tp_len = als_size(context_tuple->mr_ls);
		for (i = 0; i < tp_len; i++)
		{
			date_mr = als_get(context_tuple->mr_ls, i);
			if (date_mr->dim_role != NULL && date_mr->dim_role->gid == date_dim_role->gid)
				break;
			date_mr = NULL;
		}

		if (date_mr->member_formula || (date_mr->member->p_gid == 0))
			return NULL;

		Member *parent_mbr = find_member_by_gid(date_mr->member->p_gid);
		Member *prev = Member_same_lv_m(parent_mbr, -1);

		if (prev == NULL)
			return NULL;

		int child_posi = Member_child_position(parent_mbr, date_mr->member);
		Member *parallel_mbr = Member_get_posi_child(prev, child_posi);

		if (parallel_mbr == NULL)
			return NULL;

		return mdd_mr__create(parallel_mbr, date_dim_role);
	}

	// ParallelPeriod(<level expression>)
	if (pp->index == NULL)
	{

		LevelRole *lv_role = LevelRoleDef_interpret(md_ctx, pp->lvr_def, context_tuple, cube);

		MddMemberRole *mr = NULL;

		int i, tp_len = als_size(context_tuple->mr_ls);
		for (i = 0; i < tp_len; i++)
		{
			mr = als_get(context_tuple->mr_ls, i);
			if (mr->dim_role && mr->dim_role->gid == lv_role->dim_role->gid)
				break;
			mr = NULL;
		}

		if (mr->member->lv < lv_role->lv->level)
			return NULL;

		if (mr->member->lv == lv_role->lv->level)
		{
			return mdd_mr__create(Member_same_lv_m(mr->member, -1), lv_role->dim_role);
		}

		Member *ancestor = Member_find_ancestor(mr->member, mr->member->lv - lv_role->lv->level);

		ArrayList *desc_posi = Member_descendant_position(ancestor, mr->member);

		Member *ancestor_prev = Member_same_lv_m(ancestor, -1);

		return mdd_mr__create(Member_find_posi_descmbr(ancestor_prev, desc_posi), lv_role->dim_role);
	}

	// ParallelPeriod(<level expression>, offset)
	if (pp->mr_def == NULL)
	{

		LevelRole *lv_role = LevelRoleDef_interpret(md_ctx, pp->lvr_def, context_tuple, cube);

		MddMemberRole *mr = NULL;

		int i, tp_len = als_size(context_tuple->mr_ls);
		for (i = 0; i < tp_len; i++)
		{
			mr = als_get(context_tuple->mr_ls, i);
			if (mr->dim_role && mr->dim_role->gid == lv_role->dim_role->gid)
				break;
			mr = NULL;
		}

		if (mr->member->lv < lv_role->lv->level)
			return NULL;

		GridData prev_offset;
		Expression_evaluate(md_ctx, pp->index, cube, context_tuple, &prev_offset);

		int offset = prev_offset.val;

		if (mr->member->lv == lv_role->lv->level)
		{
			return mdd_mr__create(Member_same_lv_m(mr->member, 0 - offset), lv_role->dim_role);
		}

		unsigned int distance = mr->member->lv - lv_role->lv->level;
		Member *ancestor = Member_find_ancestor(mr->member, distance);

		ArrayList *desc_posi = Member_descendant_position(ancestor, mr->member);

		Member *ancestor_prev = Member_same_lv_m(ancestor, 0 - offset);

		return mdd_mr__create(Member_find_posi_descmbr(ancestor_prev, desc_posi), lv_role->dim_role);
	}

	// ParallelPeriod(<level expression>, offset, <member expression>)

	LevelRole *lv_role = LevelRoleDef_interpret(md_ctx, pp->lvr_def, context_tuple, cube);

	MddMemberRole *mr = ids_mbrsdef__build(md_ctx, pp->mr_def, context_tuple, cube);

	if (mr->member->lv < lv_role->lv->level)
		return NULL;

	GridData prev_offset;
	Expression_evaluate(md_ctx, pp->index, cube, context_tuple, &prev_offset);
	int offset = prev_offset.val;

	if (mr->member->lv == lv_role->lv->level)
	{
		return mdd_mr__create(Member_same_lv_m(mr->member, 0 - offset), lv_role->dim_role);
	}

	unsigned int distance = mr->member->lv - lv_role->lv->level;
	Member *ancestor = Member_find_ancestor(mr->member, distance);

	ArrayList *desc_posi = Member_descendant_position(ancestor, mr->member);

	Member *ancestor_prev = Member_same_lv_m(ancestor, 0 - offset);

	return mdd_mr__create(Member_find_posi_descmbr(ancestor_prev, desc_posi), lv_role->dim_role);
}

MddMemberRole *MemberRoleFnClosingPeriod_evolving(MDContext *md_ctx, MemberRoleFnClosingPeriod *cp, MddTuple *context_tuple, Cube *cube) {
	if (cp->lvr_def == NULL && cp->mr_def == NULL) {
		ArrayList *roles_of_date_dims = Cube_find_date_dim_roles(cube);
		if (als_size(roles_of_date_dims) != 1)
			return NULL;

		DimensionRole *date_dim_role = als_get(roles_of_date_dims, 0);
		Level *level = NULL;
		Level *lv = NULL;
		for (int i=0; i<als_size(levels_pool); i++) {
			lv = als_get(levels_pool, i);
			if (lv->dim_gid != date_dim_role->dim_gid || lv->level < 1)
				continue;

			if (level == NULL)
				level = lv;
			else if (lv->level < level->level)
				level = lv;
		}

		Member *member = NULL;
		for (int i=0; i<als_size(member_pool); i++) {
			Member *m = als_get(member_pool, i);

			if (m->dim_gid != level->dim_gid || m->lv != level->level)
				continue;
			
			if (member == NULL)
				member = m;
			else if (m->gid > member->gid)
				member = m;
		}

		return mdd_mr__create(member, date_dim_role);
	}

	if (cp->lvr_def != NULL && cp->mr_def == NULL) {
		LevelRole *lv_role = LevelRoleDef_interpret(md_ctx, cp->lvr_def, context_tuple, cube);
		Member *member = NULL;
		for (int i=0; i<als_size(member_pool); i++) {
			Member *m = als_get(member_pool, i);
			if (m->dim_gid != lv_role->dim_role->dim_gid || m->lv != lv_role->lv->level)
				continue;
			
			if (member == NULL)
				member = m;
			else if (m->gid > member->gid)
				member = m;
		}
		return mdd_mr__create(member, lv_role->dim_role);
	}

	if (cp->lvr_def != NULL && cp->mr_def != NULL) {
		LevelRole *lv_role = LevelRoleDef_interpret(md_ctx, cp->lvr_def, context_tuple, cube);
		MddMemberRole *m_role = ids_mbrsdef__build(md_ctx, cp->mr_def, context_tuple, cube);
		ArrayList *descendants = Member__descendants(m_role->member);
		Member *member = NULL;
		for (int i=0; i < als_size(descendants); i++) {
			Member *m = als_get(descendants, i);
			if (m->lv != lv_role->lv->level)
				continue;
			if (member == NULL || m->gid > member->gid)
				member = m;
		}
		return mdd_mr__create(member, lv_role->dim_role);
	}

	log_print("[ error ] MemberRoleFnClosingPeriod_evolving\n");
	exit(EXIT_FAILURE);
}

MddMemberRole *MemberRoleFnOpeningPeriod_evolving(MDContext *md_ctx, MemberRoleFnOpeningPeriod *op, MddTuple *context_tuple, Cube *cube) {
	if (op->lvr_def == NULL && op->mr_def == NULL) {
		ArrayList *roles_of_date_dims = Cube_find_date_dim_roles(cube);
		if (als_size(roles_of_date_dims) != 1)
			return NULL;

		DimensionRole *date_dim_role = als_get(roles_of_date_dims, 0);
		Level *level = NULL;
		Level *lv = NULL;
		for (int i=0; i<als_size(levels_pool); i++) {
			lv = als_get(levels_pool, i);
			if (lv->dim_gid != date_dim_role->dim_gid || lv->level < 1)
				continue;

			if (level == NULL)
				level = lv;
			else if (lv->level < level->level)
				level = lv;
		}

		Member *member = NULL;
		for (int i=0; i<als_size(member_pool); i++) {
			Member *m = als_get(member_pool, i);
			if (m->dim_gid != level->dim_gid || m->lv != level->level)
				continue;
			
			if (member == NULL)
				member = m;
			else if (m->gid < member->gid)
				member = m;
		}

		return mdd_mr__create(member, date_dim_role);
	}

	if (op->lvr_def != NULL && op->mr_def == NULL) {
		LevelRole *lv_role = LevelRoleDef_interpret(md_ctx, op->lvr_def, context_tuple, cube);
		Member *member = NULL;
		for (int i=0; i<als_size(member_pool); i++) {
			Member *m = als_get(member_pool, i);
			if (m->dim_gid != lv_role->dim_role->dim_gid || m->lv != lv_role->lv->level)
				continue;
			
			if (member == NULL)
				member = m;
			else if (m->gid < member->gid)
				member = m;
		}
		return mdd_mr__create(member, lv_role->dim_role);
	}

	if (op->lvr_def != NULL && op->mr_def != NULL) {
		LevelRole *lv_role = LevelRoleDef_interpret(md_ctx, op->lvr_def, context_tuple, cube);
		MddMemberRole *m_role = ids_mbrsdef__build(md_ctx, op->mr_def, context_tuple, cube);
		ArrayList *descendants = Member__descendants(m_role->member);
		Member *member = NULL;
		for (int i=0; i < als_size(descendants); i++) {
			Member *m = als_get(descendants, i);
			if (m->lv != lv_role->lv->level)
				continue;
			if (member == NULL || m->gid < member->gid)
				member = m;
		}
		return mdd_mr__create(member, lv_role->dim_role);
	}

	log_print("[ error ] MemberRoleFnOpeningPeriod_evolving\n");
	exit(EXIT_FAILURE);
}


MddMemberRole *MemberRoleFnFirstChild_evolving(MDContext *md_ctx, MemberRoleFnFirstChild *mr_fn, MddTuple *context_tuple, Cube *cube) {
	MddMemberRole *parent_mr = ids_mbrsdef__build(md_ctx, mr_fn->mr_def, context_tuple, cube);
	Member *member = NULL;
	for (int i=0; i<als_size(member_pool); i++) {
		Member *m = als_get(member_pool, i);
		if (m->p_gid != parent_mr->member->gid)
			continue;
		if (member == NULL || m->gid < member->gid)
			member = m;
	}
	return mdd_mr__create(member, parent_mr->dim_role);
}

MddMemberRole *MemberRoleFnLastChild_evolving(MDContext *md_ctx, MemberRoleFnLastChild *mr_fn, MddTuple *context_tuple, Cube *cube) {
	MddMemberRole *parent_mr = ids_mbrsdef__build(md_ctx, mr_fn->mr_def, context_tuple, cube);
	Member *member = NULL;
	for (int i=0; i<als_size(member_pool); i++) {
		Member *m = als_get(member_pool, i);
		if (m->p_gid != parent_mr->member->gid)
			continue;
		if (member == NULL || m->gid > member->gid)
			member = m;
	}
	return mdd_mr__create(member, parent_mr->dim_role);
}


MddMemberRole *MemberRoleFnFirstSibling_evolving(MDContext *md_ctx, MemberRoleFnFirstSibling *mr_fn, MddTuple *context_tuple, Cube *cube) {
	MddMemberRole *member_role = ids_mbrsdef__build(md_ctx, mr_fn->mr_def, context_tuple, cube);
	// if (member_role->member->p_gid == 0)
	// 	return member_role;
	Member *member = NULL;
	for (int i=0; i<als_size(member_pool); i++) {
		Member *m = als_get(member_pool, i);
		if (m->p_gid != member_role->member->p_gid)
			continue;
		if (member == NULL || m->gid <= member->gid)
			member = m;
	}
	return mdd_mr__create(member, member_role->dim_role);
}

MddMemberRole *MemberRoleFnLastSibling_evolving(MDContext *md_ctx, MemberRoleFnLastSibling *mr_fn, MddTuple *context_tuple, Cube *cube) {
	MddMemberRole *member_role = ids_mbrsdef__build(md_ctx, mr_fn->mr_def, context_tuple, cube);
	// if (member_role->member->p_gid == 0)
	// 	return member_role;
	Member *member = NULL;
	for (int i=0; i<als_size(member_pool); i++) {
		Member *m = als_get(member_pool, i);
		if (m->p_gid != member_role->member->p_gid)
			continue;
		if (member == NULL || m->gid >= member->gid)
			member = m;
	}
	return mdd_mr__create(member, member_role->dim_role);
}


int __mr_fn_lag_cmp__(void *obj, void *other) {
	Member *mobj = (Member *)obj;
	Member *moth = (Member *)other;
	return moth->gid < mobj->gid ? -1 : (moth->gid > mobj->gid ? 1 : 0);
}

MddMemberRole *MemberRoleFnLag_evolving(MDContext *md_ctx, MemberRoleFnLag *mr_fn, MddTuple *context_tuple, Cube *cube) {
	MddMemberRole *member_role = ids_mbrsdef__build(md_ctx, mr_fn->mr_def, context_tuple, cube);
	if (mr_fn->index == 0)
		return member_role;

	ArrayList *list = als_new(64, "Member *", THREAD_MAM, NULL);

	for (int i=0; i<als_size(member_pool); i++) {
		Member *m = als_get(member_pool, i);
		if (m->p_gid == member_role->member->p_gid)
			als_add(list, m);
	}

	ArrayList_sort(list, __mr_fn_lag_cmp__);

	int m_idx = 0;
	for (int i=0; i<als_size(list); i++) {
		Member *m = als_get(list, i);
		if (m->gid == member_role->member->gid) {
			m_idx = i;
			break;
		}
	}

	m_idx -= mr_fn->index;

	if (m_idx < 0) {
		m_idx = 0;
	} else if (m_idx >= als_size(list)) {
		m_idx = als_size(list) - 1;
	}

	return mdd_mr__create(als_get(list, m_idx), member_role->dim_role);
}


MultiDimResult *MultiDimResult_creat()
{
	return mam_alloc(sizeof(MultiDimResult), OBJ_TYPE__MultiDimResult, NULL, 0);
}

void MultiDimResult_print(MultiDimResult *md_rs)
{
	log_print("\n\n\n");
	log_print("### !!! MultiDimResult print( %p ) ----------------------------------------------------------------------------\n", NULL);

	if (md_rs == NULL)
	{
		log_print("##################################################################\n");
		log_print("##################################################################\n");
		log_print("##                    MultiDimResult is NULL                    ##\n");
		log_print("##################################################################\n");
		log_print("##################################################################\n");
		goto end;
	}

	int i, x_sz = als_size(md_rs->axes);
	for (i = 0; i < x_sz; i++)
	{
		MddAxis *axis = als_get(md_rs->axes, i);
		log_print("axis->posi [ %u ]\n", axis->posi);
	}
	if (x_sz != 2)
	{
		for (i = 0; i < 10; i++)
			log_print("***************************************************\n");
		goto end;
	}

	MddAxis *col_ax = als_get(md_rs->axes, 0);
	MddAxis *row_ax = als_get(md_rs->axes, 1);
	if (col_ax->posi > row_ax->posi)
	{
		MddAxis *x_tmp = col_ax;
		col_ax = row_ax;
		row_ax = x_tmp;
	}

	int col_len = als_size(col_ax->set->tuples);
	int row_len = als_size(row_ax->set->tuples);
	int col_thickness = als_size(((MddTuple *)als_get(col_ax->set->tuples, 0))->mr_ls);
	for (i = 1; i < als_size(col_ax->set->tuples); i++)
	{
		if (als_size(((MddTuple *)als_get(col_ax->set->tuples, i))->mr_ls) > col_thickness)
			col_thickness = als_size(((MddTuple *)als_get(col_ax->set->tuples, i))->mr_ls);
	}
	int row_thickness = als_size(((MddTuple *)als_get(row_ax->set->tuples, 0))->mr_ls);
	for (i = 1; i < als_size(row_ax->set->tuples); i++)
	{
		if (als_size(((MddTuple *)als_get(row_ax->set->tuples, i))->mr_ls) > row_thickness)
			row_thickness = als_size(((MddTuple *)als_get(row_ax->set->tuples, i))->mr_ls);
	}

	int ri, ci;
	log_print("\n");
	for (ri = 0; ri < col_thickness + row_len; ri++)
	{
		for (ci = 0; ci < row_thickness + col_len; ci++)
		{
			if (ri < col_thickness && ci < row_thickness)
			{
				log_print("% 20s", "-");
			}
			else if (ri < col_thickness && ci >= row_thickness)
			{
				MddTuple *c_tuple = als_get(col_ax->set->tuples, ci - row_thickness);

				if (ri < (col_thickness - als_size(c_tuple->mr_ls)))
				{
					log_print("% 20s", "[]");
					continue;
				}

				MddMemberRole *c_mr = als_get(c_tuple->mr_ls, ri - (col_thickness - als_size(c_tuple->mr_ls)));
				if (c_mr->member)
					log_print("% 20s", c_mr->member->name);
				else
					log_print("% 20s", als_get(c_mr->member_formula->path, als_size(c_mr->member_formula->path) - 1));
			}
			else if (ri >= col_thickness && ci < row_thickness)
			{
				MddTuple *r_tuple = als_get(row_ax->set->tuples, ri - col_thickness);

				if (ci < (row_thickness - als_size(r_tuple->mr_ls)))
				{
					log_print("% 20s", "[]");
					continue;
				}

				MddMemberRole *r_mr = als_get(r_tuple->mr_ls, ci - (row_thickness - als_size(r_tuple->mr_ls)));
				if (r_mr->member)
					log_print("% 20s", r_mr->member->name);
				else
					log_print("% 20s", als_get(r_mr->member_formula->path, als_size(r_mr->member_formula->path) - 1));
			}
			else if (ri >= col_thickness && ci >= row_thickness)
			{
				if (md_rs->null_flags[(ri - col_thickness) * col_len + (ci - row_thickness)])
					log_print("% 20c", '-');
				else
					log_print("% 20.2lf", md_rs->vals[(ri - col_thickness) * col_len + (ci - row_thickness)]);
			}
		}
		log_print("\n");
	}
	log_print("\n");

end:
	log_print("### ??? MultiDimResult print( %p ) ----------------------------------------------------------------------------\n", NULL);
	log_print("\n\n\n");
	fflush(stdout);
}

int MddAxis_cmp(void *obj, void *other)
{
	// Because of the aggregation priority, the one with the large position should be ranked before.
	int obj_posi = ((MddAxis *)obj)->posi;
	int oth_posi = ((MddAxis *)other)->posi;
	// log_print("[ debug ] - ((MddAxis *)obj)->posi - ((MddAxis *)other)->posi = %d\n", obj_posi - oth_posi);
	return obj_posi - oth_posi;
}

Cube *Tuple_ctx_cube(MddTuple *tuple)
{
	int i, len = als_size(tuple->mr_ls);
	for (i = 0; i < len; i++)
	{
		MddMemberRole *mr_0 = als_get(tuple->mr_ls, i);
		if (mr_0->dim_role)
			return find_cube_by_gid(mr_0->dim_role->cube_gid);
	}
	return NULL;
}

void ExpFnSum_evolving(MDContext *md_ctx, ExpFnSum *sum, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{
	grid_data->null_flag = 1;
	grid_data->val = 0;

	MddSet *set = ids_setdef__build(md_ctx, sum->set_def, ctx_tuple, cube);
	int i, sz = als_size(set->tuples);
	for (i = 0; i < sz; i++)
	{
		MddTuple *tuple = als_get(set->tuples, i);
		tuple = tuple__merge(ctx_tuple, tuple);
		GridData tmp;
		if (sum->exp)
			Expression_evaluate(md_ctx, sum->exp, cube, tuple, &tmp);
		else
			do_calculate_measure_value(md_ctx, cube, tuple, &tmp);

		if (tmp.null_flag == 0)
		{
			grid_data->val += tmp.val;
			grid_data->null_flag = 0;
		}
		else
		{
			grid_data->null_flag = 1;
			return;
		}
	}
}

void ExpFnCount_evolving(MDContext *md_ctx, ExpFnCount *count, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{

	grid_data->null_flag = 0;
	grid_data->val = 0;

	MddSet *set = ids_setdef__build(md_ctx, count->set_def, ctx_tuple, cube);

	int i, tuples_size = als_size(set->tuples);

	if (count->include_empty)
	{
		grid_data->val = tuples_size;
		return;
	}

	MddTuple **tuples_matrix_h = mam_alloc(sizeof(MddTuple *) * tuples_size, OBJ_TYPE__RAW_BYTES, NULL, 0);

	for (i = 0; i < tuples_size; i++)
	{
		tuples_matrix_h[i] = tuple__merge(ctx_tuple, (MddTuple *)(als_get(set->tuples, i)));
	}

	char *null_flags;
	double *vals = vce_vactors_values(md_ctx, tuples_matrix_h, tuples_size, &null_flags);

	for (i = 0; i < tuples_size; i++)
	{
		if (null_flags[i] == 0)
			grid_data->val += 1;
	}
}

void ExpFnLookUpCube_evolving(MDContext *md_ctx, ExpFnLookUpCube *luc, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{
	if (luc->exp_str)
	{
		Stack stk;
		char *flag_exp = mam_alloc(strlen(luc->exp_str) + strlen("@@EXP ") + 1, OBJ_TYPE__RAW_BYTES, NULL, 0);
		sprintf(flag_exp, "@@EXP %s", luc->exp_str);
		parse_mdx(flag_exp, &stk);
		stack_pop(&stk, (void **)&(luc->exp));
	}

	Cube *cubeLinked = find_cube_by_name(luc->cube_name);

	if (cubeLinked == NULL)
	{
		MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
		thrd_mam->exception_desc = "exception at ExpFnLookUpCube_evolving: unknown cube.";
		longjmp(thrd_mam->excep_ctx_env, -1);
	}

	Expression_evaluate(NULL, luc->exp, cubeLinked, cube__basic_ref_vector(cubeLinked), grid_data);
}

void ExpFnIif_evolving(MDContext *md_ctx, ExpFnIif *iif, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{
	BooleanExpression_evaluate(md_ctx, iif->bool_exp, cube, ctx_tuple, grid_data);
	Expression_evaluate(md_ctx, grid_data->boolean == GRIDDATA_BOOL_TRUE ? iif->exp1 : iif->exp2, cube, ctx_tuple, grid_data);
}

void ExpFnCoalesceEmpty_evolving(MDContext *md_ctx, ExpFnCoalesceEmpty *ce, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data)
{
	int i, exp_count = als_size(ce->exp_ls);
	for (i = 0; i < exp_count; i++)
	{
		Expression *exp = als_get(ce->exp_ls, i);
		Expression_evaluate(md_ctx, exp, cube, ctx_tuple, grid_data);
		if (grid_data->null_flag == 0)
			break;
	}
}


void *up_evolving(MDContext *md_ctx, MDMEntityUniversalPath *up, Cube *cube, MddTuple *ctx_tuple) {
	// up->list;
	void *pointer = als_get(up->list, 0);

	short _type;
	enum_oms _strat;
	MemAllocMng *_mam;
	obj_info(pointer, &_type, &_strat, &_mam);

	void *entity = NULL;

	if (_type == OBJ_TYPE__MdmEntityUpSegment) {
		// entity = upseg_evolving(md_ctx, (MdmEntityUpSegment *)pointer, cube, ctx_tuple);

		MdmEntityUpSegment *up_seg = pointer;

log_print("[ debug ] --------------------------------- up_evolving ~ OBJ_TYPE__MdmEntityUpSegment\n");
		if (als_size(up->list) == 1 && up_seg->type == MEU_SEG_TYPE_TXT) {
			int i, sz = als_size(md_ctx->select_def->set_formulas);
			for (i = 0; i < sz; i++) {
				SetFormula *sf = als_get(md_ctx->select_def->set_formulas, i);
				if (strcmp(up_seg->info.seg_str, sf->var_block) == 0) {
					return ids_setdef__build(md_ctx, sf->set_def, ctx_tuple, cube);
				}
			}
		}
	} else if (_type == OBJ_TYPE__MemberDef) {
		MddMemberRole *mr = ids_mbrsdef__build(md_ctx, (MemberDef *)pointer, ctx_tuple, cube);
		entity = mr;
	} else if (_type == OBJ_TYPE__SetFnChildren) {
		MddSet *set = SetFnChildren_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnMembers) {
		MddSet *set = SetFnMembers_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnCrossJoin) {
		MddSet *set = SetFnCrossJoin_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnFilter) {
		MddSet *set = SetFnFilter_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnLateralMembers) {
		MddSet *set = SetFnLateralMembers_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnOrder) {
		MddSet *set = SetFnOrder_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnTopCount) {
		MddSet *set = SetFnTopCount_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnExcept) {
		MddSet *set = SetFnExcept_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnYTD) {
		MddSet *set = SetFnYTD_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnDescendants) {
		MddSet *set = SetFnDescendants_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnTail) {
		MddSet *set = SetFnTail_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnBottomOrTopPercent) {
		MddSet *set = SetFnBottomOrTopPercent_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnUnion) {
		MddSet *set = SetFnUnion_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else if (_type == OBJ_TYPE__SetFnIntersect) {
		MddSet *set = SetFnIntersect_evolving(md_ctx, pointer, cube, ctx_tuple);
		entity = set;
		
	} else {
		log_print("[ error ] The up_evolving function error.\n");
		exit(EXIT_FAILURE);
	}

	if (als_size(up->list) == 1)
		return entity;
}


LevelRole *LevelRole_creat(Level *lv, DimensionRole *dr)
{
	LevelRole *lr = mam_alloc(sizeof(LevelRole), OBJ_TYPE__LevelRole, NULL, 0);
	lr->lv = lv;
	lr->dim_role = dr;
	return lr;
}

LevelRole *LevelRoleDef_interpret(MDContext *md_ctx, LevelRoleDef *lr_def, MddTuple *context_tuple, Cube *cube)
{

	int len = als_size(lr_def->lr_path);
	if (len < 2)
		return NULL;

	char *dim_role_name = als_get(lr_def->lr_path, 0);
	char *level_name = als_get(lr_def->lr_path, 1);

	DimensionRole *dr = cube__dim_role(cube, dim_role_name);

	int i, lvs_count = als_size(levels_pool);
	for (i = 0; i < lvs_count; i++)
	{
		Level *lv = als_get(levels_pool, i);
		if ((lv->dim_gid == dr->dim_gid) && (strcmp(level_name, lv->name) == 0))
		{
			return LevelRole_creat(lv, dr);
		}
	}

	return NULL;
}

ArrayList *mdd__lv_ancestor_peer_descendants(Level *ancestor_lv, Member *member)
{
	Member *member_anc = Member_find_ancestor(member, member->lv - ancestor_lv->level);
	int i, sz = als_size(member_pool);
	ArrayList *peer_descendants = als_new(64, "Member *", THREAD_MAM, NULL);
	for (i = 0; i < sz; i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->dim_gid != member->dim_gid || m->lv != member->lv)
			continue;

		Member *m_anc = Member_find_ancestor(m, m->lv - ancestor_lv->level);
		if (member_anc->gid == m_anc->gid)
			als_add(peer_descendants, m);
	}
	return peer_descendants;
}

void mdrs_to_str(MultiDimResult *md_rs, char *_cont_buf, size_t buf_len)
{

	char *cont_buf = _cont_buf;

	// sprintf(cont_buf, "\n\n\n");
	// cont_buf += strlen(cont_buf);
	// sprintf(cont_buf, "### !!! MultiDimResult print( %p ) ----------------------------------------------------------------------------\n", NULL);
	// cont_buf += strlen(cont_buf);

	if (md_rs == NULL)
	{
		sprintf(cont_buf, "##################################################################\n");
		cont_buf += strlen(cont_buf);
		sprintf(cont_buf, "##################################################################\n");
		cont_buf += strlen(cont_buf);
		sprintf(cont_buf, "##                    MultiDimResult is NULL                    ##\n");
		cont_buf += strlen(cont_buf);
		sprintf(cont_buf, "##################################################################\n");
		cont_buf += strlen(cont_buf);
		sprintf(cont_buf, "##################################################################\n");
		cont_buf += strlen(cont_buf);
		goto end;
	}

	int i, x_sz = als_size(md_rs->axes);
	// for (i = 0; i < x_sz; i++)
	// {
	// 	MddAxis *axis = als_get(md_rs->axes, i);
	// 	sprintf(cont_buf, "axis->posi [ %u ]\n", axis->posi);
	// 	cont_buf += strlen(cont_buf);
	// }
	if (x_sz != 2)
	{
		for (i = 0; i < 10; i++)
		{
			sprintf(cont_buf, "***************************************************\n");
			cont_buf += strlen(cont_buf);
		}
		goto end;
	}

	MddAxis *col_ax = als_get(md_rs->axes, 0);
	MddAxis *row_ax = als_get(md_rs->axes, 1);
	if (col_ax->posi > row_ax->posi)
	{
		MddAxis *x_tmp = col_ax;
		col_ax = row_ax;
		row_ax = x_tmp;
	}

	int col_len = als_size(col_ax->set->tuples);
	int row_len = als_size(row_ax->set->tuples);
	// int col_thickness = als_size(((MddTuple *)als_get(col_ax->set->tuples, 0))->mr_ls);
	// for (i = 1; i < als_size(col_ax->set->tuples); i++)
	// {
	// 	if (als_size(((MddTuple *)als_get(col_ax->set->tuples, i))->mr_ls) > col_thickness)
	// 		col_thickness = als_size(((MddTuple *)als_get(col_ax->set->tuples, i))->mr_ls);
	// }
	// int row_thickness = als_size(((MddTuple *)als_get(row_ax->set->tuples, 0))->mr_ls);
	// for (i = 1; i < als_size(row_ax->set->tuples); i++)
	// {
	// 	if (als_size(((MddTuple *)als_get(row_ax->set->tuples, i))->mr_ls) > row_thickness)
	// 		row_thickness = als_size(((MddTuple *)als_get(row_ax->set->tuples, i))->mr_ls);
	// }

	int col_thickness = mdd_set__max_tuple_len(col_ax->set);
	int row_thickness = mdd_set__max_tuple_len(row_ax->set);

	int ri, ci;
	// sprintf(cont_buf, "\n");
	// cont_buf += strlen(cont_buf);
	for (ri = 0; ri < col_thickness + row_len; ri++)
	{
		for (ci = 0; ci < row_thickness + col_len; ci++)
		{
			if (ri < col_thickness && ci < row_thickness)
			{
				sprintf(cont_buf, "% 20s", "-");
				cont_buf += strlen(cont_buf);
			}
			else if (ri < col_thickness && ci >= row_thickness)
			{
				MddTuple *c_tuple = als_get(col_ax->set->tuples, ci - row_thickness);

				if (ri < (col_thickness - als_size(c_tuple->mr_ls)))
				{
					sprintf(cont_buf, "% 20s", "[]");
					cont_buf += strlen(cont_buf);
					continue;
				}

				MddMemberRole *c_mr = als_get(c_tuple->mr_ls, ri - (col_thickness - als_size(c_tuple->mr_ls)));
				if (c_mr->member)
				{
					sprintf(cont_buf, "% 20s", c_mr->member->name);
					cont_buf += strlen(cont_buf);
				}
				else
				{
					sprintf(cont_buf, "% 20s", als_get(c_mr->member_formula->path, als_size(c_mr->member_formula->path) - 1));
					cont_buf += strlen(cont_buf);
				}
			}
			else if (ri >= col_thickness && ci < row_thickness)
			{
				MddTuple *r_tuple = als_get(row_ax->set->tuples, ri - col_thickness);

				if (ci < (row_thickness - als_size(r_tuple->mr_ls)))
				{
					sprintf(cont_buf, "% 20s", "[]");
					cont_buf += strlen(cont_buf);
					continue;
				}

				MddMemberRole *r_mr = als_get(r_tuple->mr_ls, ci - (row_thickness - als_size(r_tuple->mr_ls)));
				if (r_mr->member)
				{
					sprintf(cont_buf, "% 20s", r_mr->member->name);
					cont_buf += strlen(cont_buf);
				}
				else
				{
					sprintf(cont_buf, "% 20s", als_get(r_mr->member_formula->path, als_size(r_mr->member_formula->path) - 1));
					cont_buf += strlen(cont_buf);
				}
			}
			else if (ri >= col_thickness && ci >= row_thickness)
			{
				if (md_rs->null_flags[(ri - col_thickness) * col_len + (ci - row_thickness)])
				{
					sprintf(cont_buf, "% 20c", '-');
					cont_buf += strlen(cont_buf);
				}
				else
				{
					sprintf(cont_buf, "% 20.2lf", md_rs->vals[(ri - col_thickness) * col_len + (ci - row_thickness)]);
					cont_buf += strlen(cont_buf);
				}
			}
		}
		sprintf(cont_buf, "\n");
		cont_buf += strlen(cont_buf);
	}
	// sprintf(cont_buf, "\n");
	// cont_buf += strlen(cont_buf);

end:
	// sprintf(cont_buf, "### ??? MultiDimResult print( %p ) ----------------------------------------------------------------------------\n", NULL);
	// cont_buf += strlen(cont_buf);
	// sprintf(cont_buf, "\n\n\n");
	// cont_buf += strlen(cont_buf);

	assert(cont_buf < (_cont_buf + buf_len));

	unsigned long used_len = (unsigned long)cont_buf - (unsigned long)_cont_buf;
	log_print("[ info ] **************** mdrs_to_str(...) >>>>>>>>>>>>>>>>>>>>>  %lu  /  %lu  %f %%\n", used_len, buf_len, used_len * 100.0 / buf_len);
}

ByteBuf *mdrs_to_bin(MultiDimResult *md_rs)
{

	ByteBuf *buf = buf__alloc(128 * 1024); // 128k
	unsigned int *capacity = buf_cutting(buf, sizeof(int));
	*(unsigned short *)buf_cutting(buf, sizeof(short)) = INTENT__MULTIDIM_RESULT_BIN;
	*(int *)buf_cutting(buf, sizeof(int)) = als_size(md_rs->axes); // AX_COUNT

	for (int i = 0; i < als_size(md_rs->axes); i++)
	{
		MddAxis *mdd_ax = als_get(md_rs->axes, i);

		int s_len = mdd_set__len(mdd_ax->set); // S_LEN
		*(int *)buf_cutting(buf, sizeof(int)) = s_len;
		int t_len = mdd_set__max_tuple_len(mdd_ax->set); // T_LEN
		*(int *)buf_cutting(buf, sizeof(int)) = t_len;

		for (int s = 0; s < s_len; s++)
		{
			MddTuple *tuple = als_get(mdd_ax->set->tuples, s);

			for (int j = 0; j < t_len - als_size(tuple->mr_ls); j++)
			{
				*(md_gid *)buf_cutting(buf, sizeof(md_gid)) = 0;
				*(char *)buf_cutting(buf, sizeof(char)) = 0;
			}

			for (int t = 0; t < als_size(tuple->mr_ls); t++)
			{
				MddMemberRole *mr = als_get(tuple->mr_ls, t);
				*(md_gid *)buf_cutting(buf, sizeof(md_gid)) = mr->member ? mr->member->gid : 0;
				char *mbr_name = mr->member ? mr->member->name : als_get(mr->member_formula->path, als_size(mr->member_formula->path) - 1);
				char *_m_name = buf_cutting(buf, strlen(mbr_name) + 1);
				memcpy(_m_name, mbr_name, strlen(mbr_name) + 1);
			}
		}
	}

	*(long *)buf_cutting(buf, sizeof(long)) = md_rs->rs_len; // RS_LEN

	double *_vals_ = buf_cutting(buf, sizeof(double) * md_rs->rs_len);
	memcpy(_vals_, md_rs->vals, sizeof(double) * md_rs->rs_len);

	double *_null_flags_ = buf_cutting(buf, sizeof(char) * md_rs->rs_len);
	memcpy(_null_flags_, md_rs->null_flags, sizeof(char) * md_rs->rs_len);

	*capacity = buf->index;

	return buf;
}

void *gce_transform(MDContext *md_ctx, GeneralChainExpression *gce, MddTuple *context_tuple, Cube *cube)
{

	MemberDef *member_def = MemberDef_creat(MEMBER_DEF__MBR_ABS_PATH);
	member_def->mbr_abs_path = gce->chain;
	MddMemberRole *member_role = ids_mbrsdef__build(md_ctx, member_def, context_tuple, cube);

	switch (gce->final_form)
	{
	case OBJ_TYPE__MddTuple:
		MddTuple *tuple = mdd_tp__create();
		mdd_tp__add_mbrole(tuple, member_role);
		return tuple;
	default:
		log_print("[ error ] exit. The program logic error is in the gce_transform function.\n");
		exit(EXIT_FAILURE);
	}
}

int tup_is_calculated(MddTuple *tuple) {

	for (int i=0; i<als_size(tuple->mr_ls); i++) {
		MddMemberRole *mr = als_get(tuple->mr_ls, i);
        if (mr->member_formula)
        {
            return 0;
        }
	}
	return 1;
}

void put_agg_task_group(long task_group_code, int max_task_grp_num, sem_t *semt) {

	ArrayList *agg_task = als_new(32, "[0..1] long, [2] sem_t *, [3..]Action *", DIRECT, NULL);
	long num_long = max_task_grp_num;
	als_add(agg_task, (void *)task_group_code);
	als_add(agg_task, (void *)num_long);
	als_add(agg_task, semt);

	pthread_mutex_lock(&agg_tasks_lock);
	als_add(agg_tasks_pool, agg_task);
	pthread_mutex_unlock(&agg_tasks_lock);
}

void agg_task_group_result(long task_group_code, double **mea_vals_p, char **null_flags_p, int *size_p) {

	ArrayList *agg_task = NULL;

	pthread_mutex_lock(&agg_tasks_lock);
	for (int i=0;i<als_size(agg_tasks_pool);i++) {
		ArrayList *als = als_get(agg_tasks_pool, i);
		if (((long) als_get(als, 0)) == task_group_code) {
			agg_task = als;
			als_rm_index(agg_tasks_pool, i);
			break;
		}
		agg_task = NULL;
	}
	pthread_mutex_unlock(&agg_tasks_lock);

	*mea_vals_p = NULL;
	*null_flags_p = NULL;

	for (int i=3;i<als_size(agg_task);i++) {
		Action *act = als_get(agg_task, i);

		char *payload = act->bytes;

		long count_of_grids = *((long *)(payload + 4+2+8+8+4+4));

		if (count_of_grids == 0) {
			obj_release(act->bytes);
			obj_release(act);
			continue;
		}

		if (*mea_vals_p == NULL) {
			*mea_vals_p = obj_alloc(sizeof(double) * count_of_grids, OBJ_TYPE__RAW_BYTES);
			*null_flags_p = obj_alloc(sizeof(char) * count_of_grids, OBJ_TYPE__RAW_BYTES);
			memset(*null_flags_p, 1, sizeof(char) * count_of_grids);
			*size_p = (int) count_of_grids;
		}

		char *idx = payload + 4+2+8+8+4+4+8;
		for (int j=0;j<count_of_grids;j++) {
			(*mea_vals_p)[j] += *((double *)idx);
			idx += sizeof(double);
		}
		for (int j=0;j<count_of_grids;j++) {
			(*null_flags_p)[j] &= *idx;
			idx += sizeof(char);
		}

		obj_release(act->bytes);
		obj_release(act);
	}

	als_release(agg_task);

}

void put_agg_task_result(Action *act) {

	char *payload = act->bytes;

	long tg_code = *(long *)(payload + 4+2+8);

	ArrayList *agg_task = NULL;

	pthread_mutex_lock(&agg_tasks_lock);

	for (int i=0;i<als_size(agg_tasks_pool);i++) {
		agg_task = als_get(agg_tasks_pool, i);
		if (((long) als_get(agg_task, 0)) == tg_code)
			break;
		agg_task = NULL;
	}

	pthread_mutex_unlock(&agg_tasks_lock);

	if (agg_task != NULL) {
		als_add(agg_task, act);

		sem_post((sem_t *)als_get(agg_task, 2));

		return;
	}

	sleep(100);

	log_print("[ error ] - in fn:put_agg_task_result ....................................\n");
	exit(EXIT_FAILURE);

}