#include <string.h>

#include "md-model.h"
#include "mdd.h"
#include "log.h"
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
void *interpret_members(void *md_ctx_, void *entity_, void *ast_members_, void *ctx_tuple_, void *cube_)
{

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

	if (obj_type_of(entity_) == OBJ_TYPE__DimensionRole)
	{
		DimensionRole *dimrole = entity_;

		ArrayList *targetls = NULL;

		if (dimrole->bin_attr & DR_MEASURE_MASK)
		{
			// dimrole is a measure dimension role
			msz = als_size(cube->measure_mbrs);
			targetls = cube->measure_mbrs;
		}
		else
		{
			// dimrole is not a measure dimension role
			targetls = member_pool;
		}

		for (int i = 0; i < msz; i++)
		{
			Member *member = als_get(targetls, i);
			if ((dimrole->bin_attr & DR_MEASURE_MASK) || member->dim_gid == dimrole->dim_gid)
			{
				MddTuple *tuple = mdd_tp__create();
				mdd_tp__add_mbrole(tuple, mdd_mr__create(member, dimrole));
				mddset__add_tuple(set, tuple);
			}
		}
	}
	else if (obj_type_of(entity_) == OBJ_TYPE__HierarchyRole)
	{
		HierarchyRole *hierole = entity_;
		for (int i = 0; i < msz; i++)
		{
			Member *member = als_get(member_pool, i);

			if (member->hierarchy_gid != hierole->hierarchy->gid)
				continue;

			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(member, hierole->dim_role));
			mddset__add_tuple(set, tuple);
		}
	}
	else if (obj_type_of(entity_) == OBJ_TYPE__LevelRole)
	{
		LevelRole *lvrole = entity_;
		for (int i = 0; i < msz; i++)
		{
			Member *member = als_get(member_pool, i);

			if (!(member->hierarchy_gid == lvrole->lv->hierarchy_gid && member->lv == lvrole->lv->level))
				continue;

			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(member, lvrole->dim_role));
			mddset__add_tuple(set, tuple);
		}
	}
	else
	{
		MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
		thrd_mam->exception_desc = "Function interpret_members throws an exception.";
		longjmp(thrd_mam->excep_ctx_env, -1);
	}

	return set;
}

// for ASTSetFunc_CrossJoin
void *interpret_crossjoin(void *md_ctx_, void *nil, void *crossjoin_, void *ctx_tuple_, void *cube_)
{

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
void *interpret_filter(void *md_ctx_, void *nil, void *filter_, void *ctx_tuple_, void *cube_)
{

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
void *interpret_lateralmembers(void *md_ctx_, void *nil, void *lateral_, void *ctx_tuple_, void *cube_)
{

	MddMemberRole *mr = up_evolving(md_ctx_, ((ASTSetFunc_LateralMembers *)lateral_)->mrole_up, cube_, ctx_tuple_);
	if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole)
	{
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
void *interpret_order(void *md_ctx_, void *nil, void *order_, void *ctx_tuple_, void *cube_)
{

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
void *interpret_topcount(void *md_ctx_, void *setdef_, void *topcount_, void *ctx_tuple_, void *cube_)
{

	ASTSetFunc_TopCount *topcount = topcount_;
	MddSet *set = NULL;
	if (setdef_ && obj_type_of(setdef_) == OBJ_TYPE__MddSet)
	{
		set = setdef_;
	}
	else
	{
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
void *interpret_except(void *md_ctx_, void *nil, void *except_, void *ctx_tuple_, void *cube_)
{

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

// for ASTSetFunc_YTD
void *interpret_ytd(void *md_ctx_, void *mrole_, void *ytd_, void *ctx_tuple_, void *cube_)
{

	ASTSetFunc_YTD *ytd = ytd_;
	MddMemberRole *date_mr;
	Dimension *date_dim;
	MddTuple *ctx_tuple = ctx_tuple_;

	if (ytd->mrole_def)
	{
		// mr = ids_mbrsdef__build(md_ctx, ytd->mbr_def, ctx_tuple, cube);
		date_mr = up_evolving(md_ctx_, ytd->mrole_def, cube_, ctx_tuple_);
		// dim = find_dim_by_gid(mr->dim_role->dim_gid);
		date_dim = find_dim_by_gid(date_mr->dim_role->dim_gid);
	}
	else
	{
		int i, sz = als_size(ctx_tuple->mr_ls);
		for (i = 0; i < sz; i++)
		{
			date_mr = als_get(ctx_tuple->mr_ls, i);

			if (date_mr->dim_role->bin_attr & DR_MEASURE_MASK)
			{
				// skip the measure dimension
				date_mr = NULL;
				continue;
			}

			date_dim = find_dim_by_gid(date_mr->dim_role->dim_gid);
			if (strcmp(date_dim->name, "Time") == 0 || strcmp(date_dim->name, "Date") == 0)
				break;

			date_mr = NULL;
		}
	}

	Level *year_lv;
	int i, sz = als_size(levels_pool);
	for (i = 0; i < sz; i++)
	{
		year_lv = als_get(levels_pool, i);
		if (year_lv->dim_gid == date_dim->gid && strcmp(year_lv->name, "Year") == 0)
			break;
		year_lv = NULL;
	}

	ArrayList *descendants = mdd__lv_ancestor_peer_descendants(year_lv, date_mr->member);
	sz = als_size(descendants);
	MddSet *result = mdd_set__create();
	for (i = 0; i < sz; i++)
	{
		MddTuple *tuple = mdd_tp__create();
		mdd_tp__add_mbrole(tuple, mdd_mr__create(als_get(descendants, i), date_mr->dim_role));
		mddset__add_tuple(result, tuple);
		if (((Member *)als_get(descendants, i))->gid == date_mr->member->gid)
			break;
	}

	return result;
}

// for ASTSetFunc_Descendants
void *interpret_descendants(void *md_ctx_, void *nil, void *desc_, void *ctx_tuple_, void *cube_)
{

	ASTSetFunc_Descendants *descfn = desc_;

	// MddMemberRole *mr = ids_mbrsdef__build(md_ctx, desc->mbr_def, ctx_tuple, cube);
	MddMemberRole *mrole = up_evolving(md_ctx_, descfn->mrole_def, cube_, ctx_tuple_);
	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
	{
		MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
		thrd_mam->exception_desc = "Function interpret_descendants throws an exception.";
		longjmp(thrd_mam->excep_ctx_env, -1);
	}

	MddSet *result = mdd_set__create();

	ArrayList *descendants = Member__descendants(mrole->member);

	if (descfn->lvrole_def == NULL && descfn->disexp == NULL)
	{

		int i, sz = als_size(descendants);
		for (i = 0; i < sz; i++)
		{
			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(als_get(descendants, i), mrole->dim_role));
			mddset__add_tuple(result, tuple);
		}
		return result;
	}

	if (descfn->lvrole_def)
	{
		LevelRole *lvrole = up_evolving(md_ctx_, descfn->lvrole_def, cube_, ctx_tuple_);
		if (!lvrole || obj_type_of(lvrole) != OBJ_TYPE__LevelRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "Function interpret_descendants throws an exception.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		int i, sz = als_size(descendants);
		for (i = 0; i < sz; i++)
		{
			Member *mbr = als_get(descendants, i);
			switch (descfn->opt)
			{
			case SELF:
				if (mbr->lv != lvrole->lv->level)
					continue;
				break;
			case AFTER:
				if (mbr->lv <= lvrole->lv->level)
					continue;
				break;
			case BEFORE:
				if (mbr->lv >= lvrole->lv->level)
					continue;
				break;
			case BEFORE_AND_AFTER:
				if (mbr->lv == lvrole->lv->level)
					continue;
				break;
			case SELF_AND_AFTER:
				if (mbr->lv < lvrole->lv->level)
					continue;
				break;
			case SELF_AND_BEFORE:
				if (mbr->lv > lvrole->lv->level)
					continue;
				break;
			case SELF_BEFORE_AFTER:
				// do nothing
				break;
			case LEAVES:
				if (mdd_mbr__is_leaf(mbr) == 0 || mbr->lv > lvrole->lv->level)
					continue;
				break;
			default:
				log_print("[ error ] program exit, cause by: worry value of set function Descendants option.\n");
				exit(1);
			}
			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(mbr, mrole->dim_role));
			mddset__add_tuple(result, tuple);
		}
		return result;
	}

	if (descfn->disexp)
	{
		GridData data;
		Expression_evaluate(md_ctx_, descfn->disexp, cube_, ctx_tuple_, &data);
		int stan_lv = mrole->member->lv + data.val;

		int i, sz = als_size(descendants);
		for (i = 0; i < sz; i++)
		{
			Member *mbr = als_get(descendants, i);
			switch (descfn->opt)
			{
			case SELF:
				if (mbr->lv != stan_lv)
					continue;
				break;
			case AFTER:
				if (mbr->lv <= stan_lv)
					continue;
				break;
			case BEFORE:
				if (mbr->lv >= stan_lv)
					continue;
				break;
			case BEFORE_AND_AFTER:
				if (mbr->lv == stan_lv)
					continue;
				break;
			case SELF_AND_AFTER:
				if (mbr->lv < stan_lv)
					continue;
				break;
			case SELF_AND_BEFORE:
				if (mbr->lv > stan_lv)
					continue;
				break;
			case SELF_BEFORE_AFTER:
				// do nothing
				break;
			case LEAVES:
				if (mdd_mbr__is_leaf(mbr) == 0 || mbr->lv > stan_lv)
					continue;
				break;
			default:
				log_print("[ error ] program exit, cause by: worry value of set function Descendants option.\n");
				exit(1);
			}
			MddTuple *tuple = mdd_tp__create();
			mdd_tp__add_mbrole(tuple, mdd_mr__create(mbr, mrole->dim_role));
			mddset__add_tuple(result, tuple);
		}
		return result;
	}
}

// for ASTSetFunc_Tail
void *interpret_tail(void *md_ctx_, void *nil, void *tail_, void *ctx_tuple_, void *cube_)
{

	// SetFnTail *tail = set_fn;
	ASTSetFunc_Tail *tail = tail_;

	MddSet *set = ids_setdef__build(md_ctx_, tail->setdef, ctx_tuple_, cube_);

	int count = 1;
	if (tail->countexp)
	{
		GridData data;
		Expression_evaluate(md_ctx_, tail->countexp, cube_, ctx_tuple_, &data);
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

// for ASTSetFunc_BottomOrTopPercent
void *interpret_bottomortoppercent(void *md_ctx_, void *nil, void *percent_, void *ctx_tuple_, void *cube_)
{
	ASTSetFunc_BottomOrTopPercent *per = percent_;
	MddSet *set = ids_setdef__build(md_ctx_, per->set, ctx_tuple_, cube_);
	GridData data;
	Expression_evaluate(md_ctx_, per->percentage, cube_, ctx_tuple_, &data);
	double global = 0, percent = data.val / 100;
	ArrayList *vals = als_new(128, "double", THREAD_MAM, NULL);
	int i, j, sz = als_size(set->tuples);
	for (i = 0; i < sz; i++)
	{
		Expression_evaluate(md_ctx_, per->exp, cube_, tuple__merge(ctx_tuple_, als_get(set->tuples, i)), &data);
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

			if (per->option == BOTTOM_PER)
			{
				if (val_a <= val_b)
					continue;
			}
			else
			{ // per->option == TOP_PER
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

// for ASTSetFunc_Union
void *interpret_union(void *md_ctx_, void *nil, void *union_, void *ctx_tuple_, void *cube_)
{
	ASTSetFunc_Union *uni = union_;
	ArrayList *tuples = als_new(64, "MddTuple *", THREAD_MAM, NULL);
	int i, j, len = als_size(uni->set_def_ls);
	for (i = len - 1; i >= 0; i--)
	{
		MddSet *set = ids_setdef__build(md_ctx_, als_get(uni->set_def_ls, i), ctx_tuple_, cube_);
		len = als_size(set->tuples);
		for (j = len - 1; j >= 0; j--)
			als_add(tuples, als_get(set->tuples, j));
	}
	MddSet *result = mdd_set__create();
	if (uni->all_opt)
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

// for ASTSetFunc_Intersect
void *interpret_intersect(void *md_ctx_, void *nil, void *intersect_, void *ctx_tuple_, void *cube_)
{
	ASTSetFunc_Intersect *inter = intersect_;
	MddSet *set_0 = ids_setdef__build(md_ctx_, als_get(inter->set_def_ls, 0), ctx_tuple_, cube_);
	if (als_size(inter->set_def_ls) < 2)
		return set_0;
	MddSet *set_1 = ids_setdef__build(md_ctx_, als_get(inter->set_def_ls, 1), ctx_tuple_, cube_);
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

// for ASTSetFunc_Distinct
void *interpret_distinct(void *md_ctx_, void *set_, void *dist_, void *ctx_tuple_, void *cube_)
{

	ASTSetFunc_Distinct *dist = dist_;
	MddSet *set = set_;

	if (!set || obj_type_of(set) != OBJ_TYPE__MddSet)
	{
		if (!dist->setdef)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_distinct.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		// set = up_evolving(md_ctx_, dist->setdef, cube_, ctx_tuple_);
		set = ids_setdef__build(md_ctx_, dist->setdef, ctx_tuple_, cube_);
		if (!set || obj_type_of(set) != OBJ_TYPE__MddSet)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_distinct.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	unsigned int tsz = als_size(set->tuples);

	ArrayList *distlist = als_new(64, "<MddTuple *>", THREAD_MAM, NULL);

	for (int i = 0; i < tsz; i++)
	{
		MddTuple *tp = als_get(set->tuples, i);
		unsigned int dlsz = als_size(distlist);
		for (int j = 0; j < dlsz; j++)
		{
			MddTuple *targ = als_get(distlist, j);
			if (Tuple__cmp(targ, tp) == 0)
				goto bk;
		}
		als_add(distlist, tp);
	bk:
		i = i;
	}

	MddSet *dist_set = mdd_set__create();
	dist_set->tuples = distlist;
	return dist_set;
}