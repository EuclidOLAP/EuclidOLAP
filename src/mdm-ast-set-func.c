#include "md-model.h"
#include "mdd.h"
#include "mdm-ast-set-func.h"

extern ArrayList *dims_pool;
extern ArrayList *hierarchies_pool;
extern ArrayList *member_pool;
extern ArrayList *cubes_pool;
extern ArrayList *levels_pool;

/*
 * for ASTSetFunc_Children
 *
 * @return set
 */
void *interpret_children(void *md_ctx_, void *mrole_, void *ast_children_, void *ctx_tuple_, void *cube_)
{

    ASTSetFunc_Children *children = ast_children_;

    MddMemberRole *mrole = mrole_;
    if (!mrole)
    {
        if (!children->mrole_sep)
        {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: A member must be specified when the Children function is called.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }
        mrole = up_evolving(md_ctx_, children->mrole_sep, cube_, ctx_tuple_);
    }

    if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
    {
        MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
        thrd_mam->exception_desc = "exception: A member must be specified when the Children function is called.";
        longjmp(thrd_mam->excep_ctx_env, -1);
    }

    MddSet *set = mdd_set__create();
    int i, sz = als_size(member_pool);
    for (i = 0; i < sz; i++)
    {
        Member *m = als_get(member_pool, i);
        if (m->p_gid == mrole->member->gid)
        {
            MddTuple *tuple = mdd_tp__create();
            mdd_tp__add_mbrole(tuple, mdd_mr__create(m, mrole->dim_role));
            mddset__add_tuple(set, tuple);
        }
    }

    return set;
}

// for ASTSetFunc_Members
void *interpret_members(void *md_ctx_, void *entity_, void *ast_members_, void *ctx_tuple_, void *cube_) {

    ASTSetFunc_Members *members = ast_members_;

	Cube *cube = cube_;

    if (!entity_)
    {
        if (!members->eup)
        {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "Function interpret_members throws an exception.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }
        entity_ = up_evolving(md_ctx_, members->eup, cube_, ctx_tuple_);
    }

    if (!entity_)
    {
        MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
        thrd_mam->exception_desc = "Function interpret_members throws an exception.";
        longjmp(thrd_mam->excep_ctx_env, -1);
    }

    int msz = als_size(member_pool);
    MddSet *set = mdd_set__create();

    if (obj_type_of(entity_) == OBJ_TYPE__DimensionRole) {
        DimensionRole *dimrole = entity_;

		ArrayList *targetls = NULL;

		if (dimrole->bin_attr & DR_MEASURE_MASK) {
			// dimrole is a measure dimension role
			msz = als_size(cube->measure_mbrs);
			targetls = cube->measure_mbrs;
		} else {
			// dimrole is not a measure dimension role
			targetls = member_pool;
		}

		for (int i=0; i<msz; i++) {
			Member *member = als_get(targetls, i);
			if ((dimrole->bin_attr & DR_MEASURE_MASK) || member->dim_gid == dimrole->dim_gid) {
				MddTuple *tuple = mdd_tp__create();
				mdd_tp__add_mbrole(tuple, mdd_mr__create(member, dimrole));
				mddset__add_tuple(set, tuple);
			}
		}

    } else if (obj_type_of(entity_) == OBJ_TYPE__HierarchyRole) {
        HierarchyRole *hierole = entity_;
        for (int i=0; i<msz; i++) {
            Member *member = als_get(member_pool, i);

            if (member->hierarchy_gid != hierole->hierarchy->gid)
                continue;

            MddTuple *tuple = mdd_tp__create();
            mdd_tp__add_mbrole(tuple, mdd_mr__create(member, hierole->dim_role));
            mddset__add_tuple(set, tuple);
        }

    } else if (obj_type_of(entity_) == OBJ_TYPE__LevelRole) {
        LevelRole *lvrole = entity_;
        for (int i=0; i<msz; i++) {
            Member *member = als_get(member_pool, i);

            if (!(member->hierarchy_gid == lvrole->lv->hierarchy_gid && member->lv == lvrole->lv->level))
                continue;

            MddTuple *tuple = mdd_tp__create();
            mdd_tp__add_mbrole(tuple, mdd_mr__create(member, lvrole->dim_role));
            mddset__add_tuple(set, tuple);
        }

    } else {
        MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
        thrd_mam->exception_desc = "Function interpret_members throws an exception.";
        longjmp(thrd_mam->excep_ctx_env, -1);
    }

	return set;
}

// for ASTSetFunc_CrossJoin
void *interpret_crossjoin(void *md_ctx_, void *nil, void *crossjoin_, void *ctx_tuple_, void *cube_) {

    ArrayList *setdefs = ((ASTSetFunc_CrossJoin *)crossjoin_)->setdefs;

	MddSet *set_ctx = ids_setdef__build(md_ctx_, als_get(setdefs, 0), ctx_tuple_, cube_);
	ArrayList *ctx_tuple_ls = set_ctx->tuples;
	int i, ls_len = als_size(setdefs);
	for (i = 1; i < ls_len; i++)
	{
		MddSet *set = ids_setdef__build(md_ctx_, als_get(setdefs, i), ctx_tuple_, cube_);
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
    join_set->tuples = ctx_tuple_ls;

	return join_set;
}

// for ASTSetFunc_Filter
void *interpret_filter(void *md_ctx_, void *nil, void *filter_, void *ctx_tuple_, void *cube_) {

    ASTSetFunc_Filter *filter = filter_;
	MddSet *result = mdd_set__create();

	MddSet *set = ids_setdef__build(md_ctx_, filter->set_def, ctx_tuple_, cube_);
	GridData data;
	int i, len = als_size(set->tuples);
	for (i = 0; i < len; i++)
	{
		MddTuple *tuple = als_get(set->tuples, i);
		BooleanExpression_evaluate(md_ctx_, filter->boolExp, cube_, tuple__merge(ctx_tuple_, tuple), &data);
		if (data.boolean == GRIDDATA_BOOL_TRUE)
			mddset__add_tuple(result, tuple);
	}
	return result;
}

// for ASTSetFunc_LateralMembers
void *interpret_lateralmembers(void *md_ctx_, void *nil, void *lateral_, void *ctx_tuple_, void *cube_) {

    MddMemberRole *mr = up_evolving(md_ctx_, ((ASTSetFunc_LateralMembers *)lateral_)->mrole_up, cube_, ctx_tuple_);
    if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole) {
        MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
        thrd_mam->exception_desc = "Function interpret_lateralmembers throws an exception.";
        longjmp(thrd_mam->excep_ctx_env, -1);
    }

    Cube *cube = cube_;

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

// for ASTSetFunc_Order
void *interpret_order(void *md_ctx_, void *nil, void *order_, void *ctx_tuple_, void *cube_) {

	ASTSetFunc_Order *order = order_;

	MddSet *set = ids_setdef__build(md_ctx_, order->setsep, ctx_tuple_, cube_);
	int i, j, sz = als_size(set->tuples);

	ArrayList *val_ls = als_new(als_size(set->tuples), "double", THREAD_MAM, NULL);
	for (i = 0; i < sz; i++)
	{
		MddTuple *tuple = als_get(set->tuples, i);
		tuple = tuple__merge(ctx_tuple_, tuple);
		GridData data;
		Expression_evaluate(md_ctx_, order->expsep, cube_, tuple, &data);

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

			if (order->option == ASC || order->option == BASC)
			{
				if (val_b < val_a)
					goto transpose;
				continue;
			}
			else
			{
				// order->option == DESC || order->option == BDESC
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

// for ASTSetFunc_TopCount
void *interpret_topcount(void *md_ctx_, void *setdef_, void *topcount_, void *ctx_tuple_, void *cube_) {

	ASTSetFunc_TopCount *topcount = topcount_;
	MddSet *set = NULL;
	if (setdef_ && obj_type_of(setdef_) == OBJ_TYPE__MddSet) {
		set = setdef_;
	} else {
		set = ids_setdef__build(md_ctx_, topcount->set_def, ctx_tuple_, cube_);
	}

	GridData data;
	Expression_evaluate(md_ctx_, topcount->count_exp, cube_, ctx_tuple_, &data);
	int count = data.val;

	int i, j, sz = als_size(set->tuples);

	if (topcount->num_exp)
	{
		ArrayList *val_ls = als_new(als_size(set->tuples), "double", THREAD_MAM, NULL);
		for (i = 0; i < sz; i++)
		{
			MddTuple *tuple = als_get(set->tuples, i);
			tuple = tuple__merge(ctx_tuple_, tuple);
			GridData data;
			Expression_evaluate(md_ctx_, topcount->num_exp, cube_, tuple, &data);
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

// for ASTSetFunc_Except
void *interpret_except(void *md_ctx_, void *nil, void *except_, void *ctx_tuple_, void *cube_) {

	ASTSetFunc_Except *except = except_;

	MddSet *set_1 = ids_setdef__build(md_ctx_, except->setdef_1, ctx_tuple_, cube_);
	MddSet *set_2 = ids_setdef__build(md_ctx_, except->setdef_2, ctx_tuple_, cube_);

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