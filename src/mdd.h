#ifndef EUCLID__MDD_H
#define EUCLID__MDD_H 1

#include "utils.h"
#include "command.h"
#include "mdx.h"

#define MD_ENTITY_NAME_BYTSZ 128

#define META_DEF_DIMS_FILE_PATH "/meta/dims"
#define META_DEF_MBRS_FILE_PATH "/meta/mbrs"
#define META_DEF_LEVELS_FILE_PATH "/meta/levels"
#define META_DEF_CUBES_FILE_PATH "/meta/cubes"

#define MEASURE_DIM_COMM_NAME "measure"

typedef long md_gid;

int mdd_init();

int mdd_load();

#define GRIDDATA_TYPE_NUM 0
#define GRIDDATA_TYPE_BOOL 1
#define GRIDDATA_BOOL_TRUE 1
#define GRIDDATA_BOOL_FALSE 0
typedef struct _Grid_Data_
{
	char null_flag;
	char type;
	char boolean;
	double val;
} GridData;

typedef struct _stct_dim_
{
	md_gid gid;
	char name[MD_ENTITY_NAME_BYTSZ];
} Dimension;

#define MDD_MEMBER__BIN_ATTR_FLAG__NON_LEAF 1

typedef struct _stct_mbr_
{
	char name[MD_ENTITY_NAME_BYTSZ];
	md_gid gid;
	md_gid p_gid;
	md_gid dim_gid;
	unsigned short lv;

	// Each binary bit represents an attribute switch.
	// lowest bit, 0 - leaf member, 1 - non-leaf member.
	int bin_attr;

	// abs_path is a data block of length 'lv * sizeof(md_gid)' bytes.
	md_gid *abs_path;
} Member;

Member *Member_same_lv_m (Member *member, int offset);

Member *Member_get_posi_child(Member *parent, int child_posi);

Member *Member_find_ancestor(Member *member, unsigned int distance);

ArrayList *Member_descendant_position(Member *ancestor, Member *descendant);

ArrayList *Member__descendants(Member *ancestor);

Member *Member_find_posi_descmbr(Member *ancestor, ArrayList *desc_posi);

// TODO parameter 'parent' may be redundant
int Member_child_position(Member *parent, Member *child);

typedef struct level_
{
	md_gid gid;
	char name[MD_ENTITY_NAME_BYTSZ];
	md_gid dim_gid;
	unsigned int level;
} Level;
Level *Level_creat(char *name_, Dimension *dim, unsigned int level_);


void Member_print(Member *);

int mdd_mbr__is_leaf(Member *);

void mdd_mbr__set_as_leaf(Member *);

int create_dims(ArrayList *dim_names);

md_gid gen_md_gid();

int create_members(ArrayList *mbrs_info_als);

Dimension *find_dim_by_name(char *dim_name);

Dimension *find_dim_by_gid(md_gid dim_gid);

Member *find_member_lv1(Dimension *dim, char *mbr_name);

Member *find_member_child(Member *parent_mbr, char *child_name);

typedef struct _euclid_cube_stct_
{
	md_gid gid;
	char name[MD_ENTITY_NAME_BYTSZ];
	ArrayList *dim_role_ls;
	Dimension *measure_dim;
	ArrayList *measure_mbrs;
} Cube;

void Cube_print(Cube *);

ArrayList *Cube_find_date_dim_roles(Cube *);

typedef struct _dim_role_stct_
{
	md_gid gid;
	char name[MD_ENTITY_NAME_BYTSZ];
	md_gid cube_gid;
	md_gid dim_gid;
	int sn; // sequence number
} DimensionRole;

void DimensionRole_print(DimensionRole *);

int build_cube(char *name, ArrayList *dim_role_ls, ArrayList *measures);

int insert_cube_measure_vals(char *cube_name, ArrayList *ls_ids_vctr_mear);

Cube *find_cube_by_name(char *cube_name);

Cube *find_cube_by_gid(md_gid);

void gen_member_gid_abs_path(Cube *cube, ArrayList *mbr_path_str, char *abs_path);

int store_measure(EuclidCommand *ec);

int distribute_store_measure(EuclidCommand *ec);

typedef struct multidimensional_result
{
	ArrayList *axes;
	double *vals;
	char *null_flags;
	unsigned long rs_len;
} MultiDimResult;
MultiDimResult *MultiDimResult_creat();

void MultiDimResult_print(MultiDimResult *);

/**
 * Assemble a multi-dimenisonal result set into a string.
 * 
 * @param md_rs
 * @param _cont_buf
 * @param buf_len
 */
void mdrs_to_str(MultiDimResult *md_rs, char *_cont_buf, size_t buf_len);

MultiDimResult *exe_multi_dim_queries(SelectDef *);

typedef struct mdd_tuple
{
	ArrayList *mr_ls;
} MddTuple;

void Tuple_print(MddTuple *);

MddTuple *mdd_tp__create();

int Tuple__cmp(MddTuple *tuple_1, MddTuple *tuple_2);

Cube *Tuple_ctx_cube(MddTuple *tuple);

typedef struct mdd_set
{
	ArrayList *tuples;
} MddSet;

typedef struct mdd_axis
{
	MddSet *set;
	unsigned short posi;
} MddAxis;

int MddAxis_cmp(void *obj, void *other);

typedef struct mdd_mbr_role
{
	Member *member;
	MemberFormula *member_formula;
	DimensionRole *dim_role;
} MddMemberRole;

void MemberRole_print(MddMemberRole *);

/**
 * When the DimensionRole parameter is empty, it indicates the measure member role.
 */
MddMemberRole *mdd_mr__create(Member *, DimensionRole *);

int MemberRole__cmp(MddMemberRole *, MddMemberRole *);

int MddMemberRole_cmp(void *mr, void *oth);

typedef struct Level_Role_
{
	Level *lv;
	DimensionRole *dim_role;
} LevelRole;
LevelRole *LevelRole_creat(Level *, DimensionRole *);

void mdd__save_level(Level *);

void mdd__use_level(Level *);

MddSet *mdd_set__create();

MddAxis *mdd_ax__create();

void mdd_tp__add_mbrole(MddTuple *, MddMemberRole *);

MddTuple *ids_setdef__head_ref_tuple(MDContext *md_ctx, SetDef *set_def, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ids_mbrsdef__build(MDContext *md_ctx, MemberDef *m_def, MddTuple *context_tuple, Cube *cube);

LevelRole *LevelRoleDef_interpret(MDContext *md_ctx, LevelRoleDef *lr_def, MddTuple *context_tuple, Cube *cube);

DimensionRole *cube__dim_role(Cube *cube, char *dim_role_name);

Member *dim__find_mbr(Dimension *dim, ArrayList *mbr_name_path);

MddSet *ids_setdef__build(MDContext *md_ctx, SetDef *set_def, MddTuple *ctx_tuple, Cube *cube);

void mddset__add_tuple(MddSet *, MddTuple *);

MddTuple *mdd_ax__get_tuple(MddAxis *, int);

MddTuple *_MddTuple__mergeTuples(MddTuple **tps, int count);

void mdd__gen_mbr_abs_path(Member *);

Member *find_member_by_gid(md_gid);

void Expression_evaluate(MDContext *md_ctx, Expression *exp, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void Term_evaluate(MDContext *md_ctx, Term *term, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void Factory_evaluate(MDContext *md_ctx, Factory *fac, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void BooleanExpression_evaluate(MDContext *md_ctx, BooleanExpression *boolExp, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void BooleanTerm_evaluate(MDContext *md_ctx, BooleanTerm *boolTerm, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void BooleanFactory_evaluate(MDContext *md_ctx, BooleanFactory *boolFac, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

MddSet *SetFnChildren_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnMembers_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnCrossJoin_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnFilter_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnLateralMembers_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnOrder_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnTopCount_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnExcept_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnYTD_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnDescendants_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnTail_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnBottomOrTopPercent_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnUnion_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddSet *SetFnIntersect_evolving(MDContext *md_ctx, void *set_fn, Cube *cube, MddTuple *ctx_tuple);

MddMemberRole *MemberFnParent_evolving(MDContext *md_ctx, MemberFnParent *fn_parent, MddTuple *context_tuple, Cube *cube);

MddMemberRole *MemberFnCurrentMember_evolving(MDContext *md_ctx, MemberFnCurrentMember *cm, MddTuple *context_tuple, Cube *cube);

MddMemberRole *MemberFnPrevMember_evolving(MDContext *md_ctx, MemberFnPrevMember *pm, MddTuple *context_tuple, Cube *cube);

MddMemberRole *MemberRoleFnParallelPeriod_evolving(MDContext *md_ctx, MemberRoleFnParallelPeriod *pp, MddTuple *context_tuple, Cube *cube);

void ExpFnSum_evolving(MDContext *md_ctx, ExpFnSum *sum, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void ExpFnCount_evolving(MDContext *md_ctx, ExpFnCount *count, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void ExpFnLookUpCube_evolving(MDContext *md_ctx, ExpFnLookUpCube *luc, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void ExpFnIif_evolving(MDContext *md_ctx, ExpFnIif *iif, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void ExpFnCoalesceEmpty_evolving(MDContext *md_ctx, ExpFnCoalesceEmpty *ce, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

// Returns a list of peer members that have a same ancestor at the same level as the current member.
ArrayList *mdd__lv_ancestor_peer_descendants(Level *, Member *);

#endif