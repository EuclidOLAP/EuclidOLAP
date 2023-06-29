#ifndef EUCLID__MDD_H
#define EUCLID__MDD_H 1

#include "utils.h"
#include "command.h"
#include "md-model.h"
#include "math.h"
#include "mdx.h"
#include "tools/elastic-byte-buffer.h"

// #define META_DEF_DIMS_FILE_PATH "/meta/dims"
// #define META_DEF_MBRS_FILE_PATH "/meta/mbrs"
// #define META_DEF_LEVELS_FILE_PATH "/meta/levels"
// #define META_DEF_CUBES_FILE_PATH "/meta/cubes"
// #define META_DEF_HIERARCHIES_FILE_PATH "/meta/hierarchies"

#define STANDARD_MEASURE_DIMENSION "Measures"

int mdd_init();

int mdd_load();

Member *Member_same_lv_m (Member *member, int offset);

Member *Member_get_posi_child(Member *parent, int child_posi);

Member *Member_find_ancestor(Member *member, unsigned int distance);

ArrayList *Member_descendant_position(Member *ancestor, Member *descendant);

ArrayList *Member__descendants(Member *ancestor);

Member *Member_find_posi_descmbr(Member *ancestor, ArrayList *desc_posi);

int Member_child_position(Member *parent, Member *child);

void Member_print(Member *);

int mdd_mbr__is_leaf(Member *);

void mdd_mbr__set_as_leaf(Member *);

/**
 * @param dim_names A list of dimension names that need to be created.
 * @param def_hie_names names of default hierarchies
 * @param result    The execution result of the function, which can be NULL.
 */
int create_dims(ArrayList *dim_names, ArrayList *def_hie_names, EuclidCommand **result);

/*
 * @param dimension
 * @param hierarchy_name
 * @return
 */
Hierarchy *create_hierarchy(Dimension *dimension, char *hierarchy_name);

Level *Level_creat(char *name_, Dimension *dim, Hierarchy *hierarchy, unsigned int level_);

md_gid gen_md_gid();

int create_members(ArrayList *mbrs_info_als);

Dimension *find_dim_by_name(char *dim_name);

Dimension *find_dim_by_gid(md_gid dim_gid);

Member *find_member_lv0(Dimension *dimension, Hierarchy *hierarchy, char *mbr_name);

Member *find_member_child(Member *parent_mbr, char *child_name);

void Cube_print(Cube *);

ArrayList *Cube_find_date_dim_roles(Cube *);

void DimensionRole_print(DimensionRole *);

int build_cube(char *name, ArrayList *dim_role_ls, ArrayList *measures);

int insert_cube_measure_vals(char *cube_name, ArrayList *ls_ids_vctr_mear, unsigned long worker_id);

Cube *find_cube_by_name(char *cube_name);

Cube *find_cube_by_gid(md_gid);

/**
 * @return 0 - normal; not 0 - mistake
 */
int gen_member_gid_abs_path(Cube *cube, MddMemberRole *mr, char *abs_path);

int store_measure(EuclidCommand *ec);

int distribute_store_measure(EuclidCommand *ec, unsigned long worker_id);

// void MultiDimResult_print(MultiDimResult *);

// /**
//  * Assemble a multi-dimenisonal result set into a string.
//  * 
//  * @param md_rs
//  * @param _cont_buf
//  * @param buf_len
//  */
// void mdrs_to_str(MultiDimResult *md_rs, char *_cont_buf, size_t buf_len);

/**
 * Convert multidimensional query result to binary format.
 */
ByteBuf *mdrs_to_bin(MultiDimResult *md_rs);

MultiDimResult *exe_multi_dim_queries(SelectDef *);

void Tuple_print(MddTuple *);

MddTuple *mdd_tp__create();

int Tuple__cmp(MddTuple *tuple_1, MddTuple *tuple_2);

Cube *Tuple_ctx_cube(MddTuple *tuple);

int MddAxis_cmp(void *obj, void *other);

void MemberRole_print(MddMemberRole *);

/**
 * When the DimensionRole parameter is empty, it indicates the measure member role.
 */
MddMemberRole *mdd_mr__create(Member *, DimensionRole *);

int MemberRole__cmp(MddMemberRole *, MddMemberRole *);

int MddMemberRole_cmp(void *mr, void *oth);

LevelRole *LevelRole_creat(Level *, DimensionRole *);

void mdd__save_level(Level *);

void mdd__use_level(Level *);

MddSet *mdd_set__create();

unsigned int mdd_set__max_tuple_len(MddSet *set);

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

Hierarchy *dim_find_hierarchy_by_name(Dimension *dim, char *hier_name);

Hierarchy *find_hierarchy(md_gid hier_id);

Member *hi_get_root_mbr(Hierarchy *hier);

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

MddMemberRole *ASTMemberFunc_Parent_evolving(MDContext *md_ctx, ASTMemberFunc_Parent *fn_parent, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ASTMemberFunc_CurrentMember_evolving(MDContext *md_ctx, ASTMemberFunc_CurrentMember *cm, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ASTMemberFunc_PrevMember_evolving(MDContext *md_ctx, ASTMemberFunc_PrevMember *pm, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ASTMemberFunc_ParallelPeriod_evolving(MDContext *md_ctx, ASTMemberFunc_ParallelPeriod *pp, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ASTMemberFunc_ClosingPeriod_evolving(MDContext *md_ctx, ASTMemberFunc_ClosingPeriod *cp, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ASTMemberFunc_OpeningPeriod_evolving(MDContext *md_ctx, ASTMemberFunc_OpeningPeriod *op, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ASTMemberFunc_FirstChild_evolving(MDContext *md_ctx, ASTMemberFunc_FirstChild *mr_fn, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ASTMemberFunc_LastChild_evolving(MDContext *md_ctx, ASTMemberFunc_LastChild *mr_fn, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ASTMemberFunc_FirstSibling_evolving(MDContext *md_ctx, ASTMemberFunc_FirstSibling *mr_fn, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ASTMemberFunc_LastSibling_evolving(MDContext *md_ctx, ASTMemberFunc_LastSibling *mr_fn, MddTuple *context_tuple, Cube *cube);

MddMemberRole *ASTMemberFunc_Lag_evolving(MDContext *md_ctx, ASTMemberFunc_Lag *mr_fn, MddTuple *context_tuple, Cube *cube);

void ExpFnSum_evolving(MDContext *md_ctx, ExpFnSum *sum, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void ExpFnCount_evolving(MDContext *md_ctx, ExpFnCount *count, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void ExpFnLookUpCube_evolving(MDContext *md_ctx, ExpFnLookUpCube *luc, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void ExpFnIif_evolving(MDContext *md_ctx, ExpFnIif *iif, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void ExpFnCoalesceEmpty_evolving(MDContext *md_ctx, ExpFnCoalesceEmpty *ce, Cube *cube, MddTuple *ctx_tuple, GridData *grid_data);

void *up_evolving(MDContext *md_ctx, MDMEntityUniversalPath *up, Cube *cube, MddTuple *ctx_tuple);

// Returns a list of peer members that have a same ancestor at the same level as the current member.
ArrayList *mdd__lv_ancestor_peer_descendants(Level *, Member *);

void *gce_transform(MDContext *md_ctx, GeneralChainExpression *gce, MddTuple *context_tuple, Cube *cube);

void put_agg_task_group(long task_group_code, int max_task_grp_num, sem_t *semt);

void agg_task_group_result(long task_group_code, double ** mea_vals_p, char **null_flags_p, int *size_p);

void put_agg_task_result(Action *act);

/*************************************************************************************
 * MddTuple(struct mdd_tuple) functions                                              *
 *************************************************************************************/
/**
 * Whether to include the calculated formula member role.
 * return 0 - include
 *        1 - exclude
 */
int tup_is_calculated(MddTuple *tuple);

/*************************************************************************************
 * Cube(struct _euclid_cube_stct_) functions                                         *
 *************************************************************************************/
Cube *select_def__get_cube(SelectDef *sd);

#endif