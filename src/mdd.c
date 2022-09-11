#include <stdio.h>
#include <string.h>
// #include <dirent.h>
#include <unistd.h> // for usleep

#include "mdd.h"
#include "mdx.h"
#include "command.h"
#include "cfg.h"
#include "net.h"
#include "vce.h"
#include "obj-type-def.h"

extern Stack YC_STC;

extern void *parse_mdx(char *mdx);

static md_gid lastest_md_gid = -1;

static ArrayList *dims_pool = NULL;
static ArrayList *member_pool = NULL;
static ArrayList *cubes_pool = NULL;
static ArrayList *levels_pool = NULL;

static Member *_create_member_lv1(Dimension *dim, char *mbr_name);
static Member *_create_member_child(Member *parent, char *child_name);

static ArrayList *select_def__build_axes(MDContext *md_ctx, SelectDef *);

static Cube *select_def__get_cube(SelectDef *);

static MddTuple *cube__basic_ref_vector(Cube *);

static MddTuple *ax_def__head_ref_tuple(MDContext *md_ctx, AxisDef *, MddTuple *, Cube *);

static MddTuple *tuple__merge(MddTuple *cxt_tuple, MddTuple *tuple_frag);

static MddAxis *ax_def__build(MDContext *md_ctx, AxisDef *, MddTuple *, Cube *);

static unsigned int mdd_ax__len(MddAxis *);

static unsigned int mdd_set__len(MddSet *);

static MddTuple *ids_tupledef__build(MDContext *md_ctx, TupleDef *t_def, MddTuple *context_tuple, Cube *cube);

int mdd_init()
{
	dims_pool = als_create(32, "dimensions pool");
	member_pool = als_create(256, "members pool | Member *");
	cubes_pool = als_create(8, "cubes pool");
	levels_pool = als_create(128, "Level *");
}

static int load_dimensions() {

    FILE *dims_file = open_file(META_DEF_DIMS_FILE_PATH, "r");
	Dimension dim;
    while (1)
    {
		if (fread(&dim, sizeof(Dimension), 1, dims_file) < 1)
			break;

		Dimension *dimension = __objAlloc__(sizeof(Dimension), OBJ_TYPE__Dimension);
		memcpy(dimension, &dim, sizeof(Dimension));
		als_add(dims_pool, dimension);
    }
    return fclose(dims_file);
}

static int load_levels() {
    FILE *levels_file = open_file(META_DEF_LEVELS_FILE_PATH, "r");
	Level level;
    while (1)
    {
		if (fread(&level, sizeof(Level), 1, levels_file) < 1)
			break;

		Level *lv = __objAlloc__(sizeof(Level), OBJ_TYPE__Level);
		memcpy(lv, &level, sizeof(Level));
		als_add(levels_pool, lv);
    }
    return fclose(levels_file);
}

static int load_members() {
    FILE *members_file = open_file(META_DEF_MBRS_FILE_PATH, "r");
	Member memb;
    while (1)
    {
		if (fread(&memb, sizeof(Member), 1, members_file) < 1)
			break;

		Member *member = __objAlloc__(sizeof(Member), OBJ_TYPE__Member);
		memcpy(member, &memb, sizeof(Member));

		/* TODO
		 * In order to eliminate the duplicate data in the dimension member file, a bad method is used here,
		 * which needs to be solved in the subsequent optimization.
		 */
		int i;
		for (i=0;i<als_size(member_pool);i++) {
			Member *m_existed = als_get(member_pool, i);
			if (m_existed->gid == member->gid) {
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
	for (i=0;i<sz;i++)
		mdd__gen_mbr_abs_path(als_get(member_pool, i));

	return 0;
}

static int load_cubes() {

	// char c_file[128];
	// memset(c_file, 0, 128);
	// getcwd(c_file, 80);
	// strcat(c_file, META_DEF_CUBES_FILE_PATH);

	FILE *cubes_fd = open_file(META_DEF_CUBES_FILE_PATH, "r");

	char cube_stru_file[128];
	md_gid cube_id;
	// ArrayList *cube_id_arr = als_create(16, "md_gid");
	while (fread((void *)&cube_id, sizeof(md_gid), 1, cubes_fd) > 0) {
		// als_add(cube_id_arr, *((void **)&cube_id));
		memset(cube_stru_file, 0, 128);
		sprintf(cube_stru_file, "/meta/cube_%lu", cube_id);

		FILE *cube_fd = open_file(cube_stru_file, "r");

		Cube *cube = __objAlloc__(sizeof(Cube), OBJ_TYPE__Cube);
		fread(cube, sizeof(Cube), 1, cube_fd);
		cube->dim_role_ls = als_create(24, "DimensionRole *");
		cube->measure_mbrs = als_create(12, "Member *");

		int i, dr_count;
		fread(&dr_count, sizeof(unsigned int), 1, cube_fd);
		for (i=0;i<dr_count;i++) {
			DimensionRole *dim_role = __objAlloc__(sizeof(DimensionRole), OBJ_TYPE__DimensionRole);
			fread(dim_role, sizeof(DimensionRole), 1, cube_fd);
			als_add(cube->dim_role_ls, dim_role);
		}

		Dimension mea_dim__;
		fread(&mea_dim__, sizeof(Dimension), 1, cube_fd);
		cube->measure_dim = find_dim_by_gid(mea_dim__.gid);

		int mea_mbrs_count;
		fread(&mea_mbrs_count, sizeof(unsigned int), 1, cube_fd);
		for (i=0;i<mea_mbrs_count;i++) {
			Member *mea_mbr = __objAlloc__(sizeof(Member), OBJ_TYPE__Member);
			fread(mea_mbr, sizeof(Member), 1, cube_fd);
			mdd__gen_mbr_abs_path(mea_mbr);
			als_add(cube->measure_mbrs, mea_mbr);
		}

		als_add(cubes_pool, cube);

    	fclose(cube_fd);
	}
	fclose(cubes_fd);
}

int mdd_load() {
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
		printf("[WARN] - dim name too long <%s>\n", dim_name);
		return NULL;
	}

	// 1 - create a dimension object.
	Dimension *dim = (Dimension *)__objAlloc__(sizeof(Dimension), OBJ_TYPE__Dimension);
	dim->gid = gen_md_gid();
	memcpy(dim->name, dim_name, strlen(dim_name));
	printf("[INFO] create dimension [ %ld ] %s\n", dim->gid, dim->name);

	// 2 - save the dim-obj into a persistent file.
	append_file_data(META_DEF_DIMS_FILE_PATH, (char *)dim, sizeof(Dimension));

	// create a root level of dimension
	Level *rootLv = Level_creat("ROOT_LEVEL", dim, 0);
	mdd__save_level(rootLv);
	mdd__use_level(rootLv);

	als_add(dims_pool, dim);
	// printf("========================= dim->name %s\n", dim->name);
	return dim;
}

int create_dims(ArrayList *dim_names)
{
	__uint32_t i, sz = als_size(dim_names);
	for (i = 0; i < sz; i++)
	{
		char *dim_name = (char *)als_get(dim_names, i);
		create_dimension(dim_name);
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

	Level *lv = __objAlloc__(sizeof(Level), OBJ_TYPE__Level);
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
	ArrayList *same_lv_ms = als_create(128, "Member *");
	for (i = 0; i < m_pool_sz; i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->dim_gid == member->dim_gid && m->lv == member->lv)
			als_add(same_lv_ms, m);
		if (m->gid == member->gid)
			curr_m_idx = als_size(same_lv_ms) - 1;
	}
	int sibling_idx = curr_m_idx + offset;
	if (sibling_idx >= 0 && sibling_idx < als_size(same_lv_ms))
		return als_get(same_lv_ms, sibling_idx);
	return NULL;
}

Member *Member_get_posi_child(Member *parent, int child_posi)
{
	int i, m_pool_sz = als_size(member_pool);
	ArrayList *children = als_create(128, "Member *");
	for (i = 0; i < m_pool_sz; i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->p_gid == parent->gid)
			als_add(children, m);
	}
	return child_posi >= 0 && child_posi < als_size(children) ? als_get(children, child_posi) : NULL;
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
	ArrayList *posi = als_create(32, "long");
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
	ArrayList *descendants = als_create(128, "Member *");
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

	Member *mbr = __objAlloc__(sizeof(Member), OBJ_TYPE__Member);
	memcpy(mbr->name, name, strlen(name));
	mbr->gid = gen_md_gid();
	mbr->dim_gid = dim_gid;
	mbr->p_gid = parent_gid;
	mbr->lv = lv;
	printf("[INFO] new Member - dim_gid [ %ld ] p_gid [% 17ld ] gid [ %ld ] name [ %s ] lv [ %d ]\n", mbr->dim_gid, mbr->p_gid, mbr->gid, mbr->name, mbr->lv);

	return mbr;
}

int build_cube(char *name, ArrayList *dim_role_ls, ArrayList *measures)
{
	if (strlen(name) >= MD_ENTITY_NAME_BYTSZ)
		return -1;

	// Create a cube object.
	Cube *cube = (Cube *)__objAlloc__(sizeof(Cube), OBJ_TYPE__Cube);
	memcpy(cube->name, name, strlen(name));
	cube->gid = gen_md_gid();
	cube->dim_role_ls = als_create(24, "DimensionRole *");
	cube->measure_mbrs = als_create(12, "Member *");
	printf("[INFO] new Cube - gid [ %ld ] name [ %s ]\n", cube->gid, cube->name);

	// Create several dimensional role objects and associate them to the cube.
	size_t i, dr_sz = als_size(dim_role_ls);
	for (i = 0; i < dr_sz; i += 2)
	{
		char *dim_name = als_get(dim_role_ls, i);
		char *dim_role_name = als_get(dim_role_ls, i + 1);
		Dimension *dim = find_dim_by_name(dim_name);

		DimensionRole *d_role = __objAlloc__(sizeof(DimensionRole), OBJ_TYPE__DimensionRole);
		d_role->sn = i / 2;
		memcpy(d_role->name, dim_role_name, strlen(dim_role_name));
		d_role->gid = gen_md_gid();
		d_role->cube_gid = cube->gid;
		d_role->dim_gid = dim->gid;
		printf("[INFO] new DimensionRole - Cube [ %ld % 16s ] Dim [ %ld % 16s ] DR [ %ld % 16s ]\n",
			   cube->gid, cube->name, dim->gid, dim->name, d_role->gid, d_role->name);

		als_add(cube->dim_role_ls, d_role);
	}

	// Create a measure dimension object.
	Dimension *mear_dim = create_dimension(MEASURE_DIM_COMM_NAME);

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
	if (d_nodes_count() < 1 || rand() % 2)
	{
		return store_measure(ec); // Store in the current node.
	}

	return send(random_child_sock(), ec->bytes, *((int *)(ec->bytes)), 0) == (ssize_t)(*((int *)(ec->bytes)));
}

// TODO [ bug? ] When inserting duplicate data, the later data will not overwrite the previous data.
int insert_cube_measure_vals(char *cube_name, ArrayList *ls_ids_vctr_mear)
{
	size_t data_m_capacity = 2 * 1024 * 1024, data_m_sz = sizeof(__uint32_t) + sizeof(__uint16_t);
	char *data = __objAlloc__(data_m_capacity, OBJ_TYPE__RAW_BYTES);
	*((__uint16_t *)(data + sizeof(__uint32_t))) = INTENT__INSERT_CUBE_MEARSURE_VALS;

	Cube *cube = find_cube_by_name(cube_name);
	*((md_gid *)(data + data_m_sz)) = cube->gid;
	data_m_sz += sizeof(md_gid);

	*((__uint32_t *)(data + data_m_sz)) = als_size(cube->dim_role_ls);
	data_m_sz += sizeof(__uint32_t);

	*((__uint32_t *)(data + data_m_sz)) = als_size(cube->measure_mbrs);
	data_m_sz += sizeof(__uint32_t);

	__uint32_t i, j, k, sz = als_size(ls_ids_vctr_mear);
	for (i = 0; i < sz; i++)
	{
		IDSVectorMears *ids_vm = als_get(ls_ids_vctr_mear, i);
		__uint32_t vct_sz = als_size(ids_vm->ls_vector);
		for (j = 0; j < vct_sz; j++)
		{
			ArrayList *mbr_path_str = als_get(ids_vm->ls_vector, j);

			__uint32_t mbr_path_len = als_size(mbr_path_str);

			void *abs_path = gen_member_gid_abs_path(cube, mbr_path_str);
			size_t ap_bsz = *((__uint32_t *)abs_path) * sizeof(md_gid) + sizeof(__uint32_t);
			memcpy(data + data_m_sz, abs_path, ap_bsz);
			data_m_sz += ap_bsz;
		}

		__uint32_t cube_mmbrs_sz = als_size(cube->measure_mbrs);
		__uint32_t mv_sz = als_size(ids_vm->ls_mears_vals);
		for (j = 0; j < cube_mmbrs_sz; j++)
		{
			Member *mm = als_get(cube->measure_mbrs, j);

			// Set the null-value flag bit, 1 means the measure-value is null.
			*(data + data_m_sz + sizeof(double)) = 1;

			for (k = 0; k < mv_sz; k++)
			{
				char *mm_name = als_get(ids_vm->ls_mears_vals, k);
				if (strcmp(mm_name, mm->name) != 0)
					continue;

				*((double *)(data + data_m_sz)) = *((double *)(als_get(ids_vm->ls_mears_vals, k + 1)));
				*(data + data_m_sz + sizeof(double)) = 0;
				break;
			}
			data_m_sz += sizeof(double) + sizeof(char);
		}
	}

	*((__uint32_t *)data) = data_m_sz; // set data package capacity
	char *_ec_data_ = __objAlloc__(data_m_sz, OBJ_TYPE__RAW_BYTES);
	memcpy(_ec_data_, data, data_m_sz);
	// _release_mem_(data);

	EuclidCommand *_ec_ = create_command(_ec_data_);

	// Store measure values locally or distribute it to downstream nodes for processing
	return distribute_store_measure(_ec_);
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

void *gen_member_gid_abs_path(Cube *cube, ArrayList *mbr_path_str)
{
	char *dim_role_name = als_get(mbr_path_str, 0);
	DimensionRole *dr;
	Dimension *dim;
	Member *lv1_mbr, *mbr;
	__uint32_t i, num_drs = als_size(cube->dim_role_ls);
	for (i = 0; i < num_drs; i++)
	{
		dr = als_get(cube->dim_role_ls, i);
		if (strcmp(dim_role_name, dr->name) != 0)
			continue;
		dim = find_dim_by_gid(dr->dim_gid);
		mbr = lv1_mbr = find_member_lv1(dim, (char *)als_get(mbr_path_str, 1));
		break;
	}

	// printf("dim_role_name [ %s ], dim->name [ %s ], lv1_mbr->name [ %s ]\n", dim_role_name, dim->name, lv1_mbr->name);

	__uint32_t sz = als_size(mbr_path_str);

	char *abs_path = __objAlloc__(sizeof(__uint32_t) + sizeof(md_gid) * (sz - 1), OBJ_TYPE__RAW_BYTES);
	*((__uint32_t *)abs_path) = sz - 1;
	*((md_gid *)(abs_path + sizeof(__uint32_t))) = lv1_mbr->gid;

	for (i = 2; i < sz; i++)
	{
		mbr = find_member_child(mbr, als_get(mbr_path_str, i));
		*((md_gid *)(abs_path + sizeof(__uint32_t) + sizeof(md_gid) * (i - 1))) = mbr->gid;
		// printf("::>> child member name - %s\n", mbr->name);
	}

	return abs_path;
}

static long query_times = 1;

MultiDimResult *exe_multi_dim_queries(SelectDef *select_def)
{
	printf("\n[ debug ] >>>>>>>>>>>>>>>>>>>>>>> The number of times the query was executed: %ld\n\n", query_times++);

	MDContext *md_ctx = MDContext_creat();
	md_ctx->select_def = select_def;

	// Build the real axes in this multidimensional query.
	ArrayList *axes = select_def__build_axes(md_ctx, select_def);

	int i;
	for (i=0;i<als_size(axes);i++) {
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

	MddTuple **tuples_matrix_h = __objAlloc__(rs_len * x_size * sizeof(void *), OBJ_TYPE__RAW_BYTES);

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

	// 'measure_vals' is equivalent to a double array whose length is 'rs_len'.
	double *measure_vals = vce_vactors_values(md_ctx, tuples_matrix_h, rs_len, &(md_result->null_flags));
	md_result->axes = axes;
	md_result->vals = measure_vals;
	md_result->rs_len = rs_len;

	// printf("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	// printf("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	// int lv_count = als_size(levels_pool);
	// for (i=0;i<lv_count;i++) {
	// 	Level *lv = als_get(levels_pool, i);
	// 	Dimension *dim = find_dim_by_gid(lv->dim_gid);
	// 	printf("% 40s    %u:%30s\n", dim->name, lv->level, lv->name);
	// }
	// printf("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	// printf("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");

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
	ArrayList *axes_ls = als_create(16, "MddAxis *");
	for (i = 0; i < ax_count; i++)
	{
		AxisDef *ax_def = als_get(ax_def_ls, i);
		MddAxis *ax = ax_def__build(md_ctx, ax_def, ref_tuple, cube);
		als_add(axes_ls, ax);
	}

	return axes_ls;
}

static Cube *select_def__get_cube(SelectDef *select_def)
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

	// printf("@@@@@@@@@@@@@@@@@@@@Tuple_print(tp);@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	// Tuple_print(tp);
	// printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
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
	MddTuple *tp = (MddTuple *)__objAlloc__(sizeof(MddTuple), OBJ_TYPE__MddTuple);
	tp->mr_ls = als_create(32, "MddMemberRole *");
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
	MddMemberRole *mr = __objAlloc__(sizeof(MddMemberRole), OBJ_TYPE__MddMemberRole);
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
		return ids_tupledef__build(md_ctx, als_get(set_def->tuple_def_ls, 0), context_tuple, cube);
	}
	else if (set_def->t_cons == SET_DEF__SET_FUNCTION)
	{
		if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SET_FN_CHILDREN)
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
			printf("[ error ] - ids_setdef__head_ref_tuple() obj_type_of(set_def->set_fn) = %d\n", obj_type_of(set_def->set_fn));
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
	else
	{
		printf("[ error ] - ids_setdef__head_ref_tuple() set_def->t_cons = %d\n", set_def->t_cons);
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

			ArrayList *mbr_path = als_create(32, "char *");
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
			else
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
			}
		}
		else
		{ // measure dimension
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

			// formula member
			int f_sz = als_size(md_ctx->select_def->member_formulas);
			for (i = 0; i < f_sz; i++)
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
		else
		{
			printf("[ error ] - ids_mbrsdef__build() obj_type_of(m_def->member_fn)\n");
			exit(1);
		}
	}
	else
	{
		printf("[error] Unknown type about defining dimension member.\n");
		exit(1);
	}
	printf("[ error ] Incorrect program execution path, causing the program to exit.\n");
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
	// printf("[warn] no DimensionRole that name is [%s]\n", dim_role_name);
	return NULL;
}

Member *dim__find_mbr(Dimension *dim, ArrayList *mbr_name_path)
{
	int i, len = als_size(mbr_name_path);
	Member *m = find_member_lv1(dim, als_get(mbr_name_path, 0));
	for (i = 1; i < len; i++)
		m = find_member_child(m, als_get(mbr_name_path, i));

	return m;
}

MddSet *mdd_set__create()
{
	MddSet *set = __objAlloc__(sizeof(MddSet), OBJ_TYPE__MddSet);
	set->tuples = als_create(64, "MddTuple *");
	return set;
}

MddAxis *mdd_ax__create()
{
	return (MddAxis *)__objAlloc__(sizeof(MddAxis), OBJ_TYPE__MddAxis);
}

MddSet *ids_setdef__build(MDContext *md_ctx, SetDef *set_def, MddTuple *ctx_tuple, Cube *cube)
{
	if (set_def->t_cons == SET_DEF__TUP_DEF_LS)
	{
		MddSet *set = mdd_set__create();
		int i, sz = als_size(set_def->tuple_def_ls);
		for (i = 0; i < sz; i++)
		{
			TupleDef *tp_def = als_get(set_def->tuple_def_ls, i);
			MddTuple *tp = ids_tupledef__build(md_ctx, tp_def, ctx_tuple, cube);
			mddset__add_tuple(set, tp);
		}
		return set;
	}
	else if (set_def->t_cons == SET_DEF__SET_FUNCTION)
	{
		if (obj_type_of(set_def->set_fn) == OBJ_TYPE__SET_FN_CHILDREN)
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
			printf("[warn] - ids_setdef__build() obj_type_of(set_def->set_fn) = %d\n", obj_type_of(set_def->set_fn));
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
	else
	{
		printf("[warn] wrong SetDef::t_cons\n");
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
	ArrayList *date_drs = als_create(8, "DimensionRole *");
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
	printf(">>> [ Cube info ] @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ addr < %p >\n", c);
	printf("\t     name - %s\n", c->name);
	printf("\t      gid - %lu\n", c->gid);
}

void Tuple_print(MddTuple *tuple)
{
	printf("{\n");
	printf("\"type\": \"Tuple\",\n");
	printf("\"mr_ls\": [\n");

	unsigned int i, len = als_size(tuple->mr_ls);
	for (i = 0; i < len; i++)
	{
		MemberRole_print(als_get(tuple->mr_ls, i));
		if (i < len - 1)
			printf(",\n");
	}

	printf("]\n");
	printf("}\n");
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
	printf("{\n");
	printf("\"type\": \"MemberRole\",\n");
	if (mr->member)
	{
		printf("\"member\": ");
		Member_print(mr->member);
	}
	else
	{
		printf("\"member_formula\": ");
		MemberFormula_print(mr->member_formula);
	}
	printf(",\n");
	printf("\"dim_role\": ");
	if (mr->dim_role)
	{
		DimensionRole_print(mr->dim_role);
	}
	else
	{
		printf("\"*** measure dimension role ***\"");
	}
	printf("}\n");
}

void Member_print(Member *m)
{
	printf("{ \"type\": \"Member\", \"name\": \"%s\" }\n", m->name);
}

void DimensionRole_print(DimensionRole *dr)
{
	if (dr)
		printf("{ \"DimRole\": \"%s\", \"sn\": %d }\n", dr->name, dr->sn);
	else
		printf("null\n");
}

void mdd__gen_mbr_abs_path(Member *m)
{
	if (m->abs_path)
		return;

	m->abs_path = __objAlloc__(m->lv * sizeof(md_gid), OBJ_TYPE__RAW_BYTES);

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
			printf("[ error ] - Factory_evaluate() - Unknown expression function type.\n");
			exit(1);
		}
	}
	else
	{
		printf("[ error ] - Factory_evaluate() <program exit>\n");
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
	{ // measure dimension role
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
		ArrayList *tuple_ls = als_create(512, "MddTuple *");
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

	ArrayList *val_ls = als_create(als_size(set->tuples), "double");
	for (i = 0; i < sz; i++)
	{
		MddTuple *tuple = als_get(set->tuples, i);
		tuple = tuple__merge(ctx_tuple, tuple);
		GridData data;
		Expression_evaluate(md_ctx, order->exp, cube, tuple, &data);
		printf("value = % 32lf, null_flag < %d >\n", data.val, data.null_flag);
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
		ArrayList *val_ls = als_create(als_size(set->tuples), "double");
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
				printf("[ error ] program exit, cause by: worry value of set function Descendants option flag < %c >\n", desc->flag);
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
				printf("[ error ] program exit, cause by: worry value of set function Descendants option flag < %c >\n", desc->flag);
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
	ArrayList *vals = als_create(128, "double");
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
	ArrayList *tuples = als_create(64, "MddTuple *");
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
		ArrayList *nonredundant = als_create(64, "MddTuple *");
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

	printf("[ error ] - MemberFnCurrentMember do not matching DimensionRole - < %s >\n", dimRole_name);
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

	printf("[ error ] - MemberFnCurrentMember do not matching DimensionRole - < %s >\n", dimRole_name);
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

MultiDimResult *MultiDimResult_creat()
{
	return __objAlloc__(sizeof(MultiDimResult), OBJ_TYPE__MultiDimResult);
}

void MultiDimResult_print(MultiDimResult *md_rs)
{
	printf("\n\n\n");
	printf("### !!! MultiDimResult print( %p ) ----------------------------------------------------------------------------\n", NULL);

	if (md_rs == NULL) {
		printf("##################################################################\n");
		printf("##################################################################\n");
		printf("##                    MultiDimResult is NULL                    ##\n");
		printf("##################################################################\n");
		printf("##################################################################\n");
		goto end;
	}

	int i, x_sz = als_size(md_rs->axes);
	for (i = 0; i < x_sz; i++)
	{
		MddAxis *axis = als_get(md_rs->axes, i);
		printf("axis->posi [ %u ]\n", axis->posi);
	}
	if (x_sz != 2)
	{
		for (i = 0; i < 10; i++)
			printf("***************************************************\n");
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
	printf("\n");
	for (ri = 0; ri < col_thickness + row_len; ri++)
	{
		for (ci = 0; ci < row_thickness + col_len; ci++)
		{
			if (ri < col_thickness && ci < row_thickness)
			{
				printf("% 20s", "-");
			}
			else if (ri < col_thickness && ci >= row_thickness)
			{
				MddTuple *c_tuple = als_get(col_ax->set->tuples, ci - row_thickness);

				if (ri < (col_thickness - als_size(c_tuple->mr_ls)))
				{
					printf("% 20s", "[]");
					continue;
				}

				MddMemberRole *c_mr = als_get(c_tuple->mr_ls, ri - (col_thickness - als_size(c_tuple->mr_ls)));
				if (c_mr->member)
					printf("% 20s", c_mr->member->name);
				else
					printf("% 20s", als_get(c_mr->member_formula->path, als_size(c_mr->member_formula->path) - 1));
			}
			else if (ri >= col_thickness && ci < row_thickness)
			{
				MddTuple *r_tuple = als_get(row_ax->set->tuples, ri - col_thickness);

				if (ci < (row_thickness - als_size(r_tuple->mr_ls)))
				{
					printf("% 20s", "[]");
					continue;
				}

				MddMemberRole *r_mr = als_get(r_tuple->mr_ls, ci - (row_thickness - als_size(r_tuple->mr_ls)));
				if (r_mr->member)
					printf("% 20s", r_mr->member->name);
				else
					printf("% 20s", als_get(r_mr->member_formula->path, als_size(r_mr->member_formula->path) - 1));
			}
			else if (ri >= col_thickness && ci >= row_thickness)
			{
				if (md_rs->null_flags[(ri - col_thickness) * col_len + (ci - row_thickness)])
					printf("% 20c", '-');
				else
					printf("% 20.2lf", md_rs->vals[(ri - col_thickness) * col_len + (ci - row_thickness)]);
			}
		}
		printf("\n");
	}
	printf("\n");

end:
	printf("### ??? MultiDimResult print( %p ) ----------------------------------------------------------------------------\n", NULL);
	printf("\n\n\n");
	fflush(stdout);
}

int MddAxis_cmp(void *obj, void *other)
{
	// Because of the aggregation priority, the one with the large position should be ranked before.
	int obj_posi = ((MddAxis *)obj)->posi;
	int oth_posi = ((MddAxis *)other)->posi;
	printf("[ debug ] - ((MddAxis *)obj)->posi - ((MddAxis *)other)->posi = %d\n", obj_posi - oth_posi);
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

	MddTuple **tuples_matrix_h = __objAlloc__(sizeof(MddTuple *) * tuples_size, OBJ_TYPE__RAW_BYTES);

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
		char *flag_exp = __objAlloc__(strlen(luc->exp_str) + 50, OBJ_TYPE__RAW_BYTES);
		sprintf(flag_exp, "@@EXP %s", luc->exp_str);
		parse_mdx(flag_exp);
		stack_pop(&YC_STC, (void **)&(luc->exp));
	}

	Cube *cubeLinked = find_cube_by_name(luc->cube_name);
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

LevelRole *LevelRole_creat(Level *lv, DimensionRole *dr)
{
	LevelRole *lr = __objAlloc__(sizeof(LevelRole), OBJ_TYPE__LevelRole);
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
	ArrayList *peer_descendants = als_create(64, "Member *");
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