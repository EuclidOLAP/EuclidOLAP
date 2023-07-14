#include <string.h>

#include "md-model.h"
#include "mdd.h"
#include "vce.h"
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

// for ASTSetFunc_DrilldownLevel
void *interpret_drilldownlevel(void *md_ctx_, void *nil, void *ddl, void *ctx_tuple_, void *cube_) {
	ASTSetFunc_DrilldownLevel *drilldl = ddl;

	MddSet *set = ids_setdef__build(md_ctx_, drilldl->setdef, ctx_tuple_, cube_);
	LevelRole *lrole = NULL;
	if (drilldl->lvrole_up) {
		lrole = up_evolving(md_ctx_, drilldl->lvrole_up, cube_, ctx_tuple_);
		if (!lrole || obj_type_of(lrole) != OBJ_TYPE__LevelRole) {
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "Function interpret_drilldownlevel throws an exception.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	int setsz = als_size(set->tuples);

	MddSet *resset = mdd_set__create();

	if (lrole) {
		for (int i=0;i<setsz;i++) {
			MddTuple *tup = als_get(set->tuples, i);
			mddset__add_tuple(resset, tup);
			int tpmrsz = als_size(tup->mr_ls);
			for (int j=0;j<tpmrsz;j++) {
				MddMemberRole *mrole = als_get(tup->mr_ls, j);
				if (mrole->dim_role->gid == lrole->dim_role->gid && mrole->member->lv == lrole->lv->level) {
					ArrayList *children = find_member_children(mrole->member);
					int chi_sz = als_size(children);
					for (int k=0;k<chi_sz;k++) {
						Member *child = als_get(children, k);
						MddMemberRole *_mr_ = mdd_mr__create(child, lrole->dim_role);
						MddTuple *chi_tup = tuple_inset_mr(tup, _mr_);
						mddset__add_tuple(resset, chi_tup);
					}
					break;
				}
			}
		}
	} else {
		int bottom_lval = 0;
		for (int i=0;i<setsz;i++) {
			MddTuple *tup = als_get(set->tuples, i);
			if (drilldl->index < als_size(tup->mr_ls)) {
				MddMemberRole *mr = als_get(tup->mr_ls, drilldl->index);
				if (mr->member->lv > bottom_lval)
					bottom_lval = mr->member->lv;
			}
		}
		
		for (int i=0;i<setsz;i++) {
			MddTuple *tup = als_get(set->tuples, i);
			mddset__add_tuple(resset, tup);

			if (drilldl->index >= als_size(tup->mr_ls))
				continue;

			MddMemberRole *mrole = als_get(tup->mr_ls, drilldl->index);
			if (mrole->member->lv == bottom_lval) {
				ArrayList *children = find_member_children(mrole->member);
				int chi_sz = als_size(children);
				for (int k=0;k<chi_sz;k++) {
					Member *child = als_get(children, k);
					MddMemberRole *_mr_ = mdd_mr__create(child, mrole->dim_role);
					MddTuple *chi_tup = tuple_inset_mr(tup, _mr_);
					mddset__add_tuple(resset, chi_tup);
				}
			}
		}
	}

	return resset;
}

void _bottop_sort_(char type, GridData *cellarr, ArrayList *memberls) {
	unsigned int sz = als_size(memberls);
	for (int i=0;i<sz-1;i++) {
		for (int j=i+1;j<sz;j++) {
			if ((type == 'b' && cellarr[i].val > cellarr[j].val) || (type == 't' && cellarr[i].val < cellarr[j].val)) {
				double var = cellarr[i].val;
				cellarr[i].val = cellarr[j].val;
				cellarr[j].val = var;

				void *addr = als_get(memberls, i);
				ArrayList_set(memberls, i, als_get(memberls, j));
				ArrayList_set(memberls, j, addr);
			}
		}
	}
}

// for ASTSetFunc_DrilldownLevelBottomTop
void *interpret_drilldownlevelbottomtop(void *md_ctx_, void *nil, void *bottop_, void *ctx_tuple_, void *cube_) {

	ASTSetFunc_DrilldownLevelBottomTop *bottop = bottop_;

	MddSet *set = ids_setdef__build(md_ctx_, bottop->setdef, ctx_tuple_, cube_);
	GridData cell;
	Expression_evaluate(md_ctx_, bottop->countexp, cube_, ctx_tuple_, &cell);
	int count = (int)cell.val;

	int setsz = als_size(set->tuples);

	MddSet *resset = mdd_set__create();

	if (!bottop->uncertainexp && !bottop->sortexp) {
		int bottom_lval = 0;
		for (int i=0;i<setsz;i++) {
			MddTuple *tup = als_get(set->tuples, i);
			MddMemberRole *mr = als_get(tup->mr_ls, 0);
			if (mr->member->lv > bottom_lval)
				bottom_lval = mr->member->lv;
		}
		
		for (int i=0;i<setsz;i++) {
			MddTuple *tup = als_get(set->tuples, i);
			mddset__add_tuple(resset, tup);

			MddMemberRole *mrole = als_get(tup->mr_ls, 0);
			if (mrole->member->lv == bottom_lval) {
				ArrayList *children = find_member_children(mrole->member);
				int chi_sz = als_size(children);
				GridData *cellarr = mam_alloc(sizeof(GridData) * chi_sz, OBJ_TYPE__RAW_BYTES, NULL, 0);
				for (int k=0;k<chi_sz;k++) {
					Member *child = als_get(children, k);
					MddMemberRole *_mr_ = mdd_mr__create(child, mrole->dim_role);
					MddTuple *chi_tup = tuple_inset_mr(tup, _mr_);
					do_calculate_measure_value(md_ctx_, cube_, tuple__merge(ctx_tuple_, chi_tup), cellarr + k);
				}
				_bottop_sort_(bottop->type, cellarr, children);
				for (int k=0;k<chi_sz && k<count;k++) {
					MddTuple *chi_tup = tuple_inset_mr(tup, mdd_mr__create(als_get(children, k), mrole->dim_role));
					mddset__add_tuple(resset, chi_tup);
				}
			}
		}
	} else if (bottop->uncertainexp && bottop->sortexp) {
		Term *term = als_get(bottop->uncertainexp->plus_terms, 0);
		Factory *fac = als_get(term->mul_factories, 0);
		MDMEntityUniversalPath *lv_up = fac->up;

		LevelRole *lvrole = up_evolving(md_ctx_, lv_up, cube_, ctx_tuple_);
		if (!lvrole || obj_type_of(lvrole) != OBJ_TYPE__LevelRole) {
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "Function interpret_drilldownlevelbottomtop throws an exception.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		for (int i=0;i<setsz;i++) {
			MddTuple *tup = als_get(set->tuples, i);
			mddset__add_tuple(resset, tup);
			MddMemberRole *mrole = NULL;
			for (int j=0;j<als_size(tup->mr_ls);j++) {
				mrole = als_get(tup->mr_ls, j);
				if (mrole->dim_role->gid == lvrole->dim_role->gid && mrole->member->lv == lvrole->lv->level) {
					break;
				} else {
					mrole = NULL;
				}
			}

			if (!mrole)
				continue;

			// if (mrole->dim_role->gid == lvrole->dim_role->gid && mrole->member->lv == lvrole->lv->level) {
			ArrayList *children = find_member_children(mrole->member);
			int chi_sz = als_size(children);
			GridData *cellarr = mam_alloc(sizeof(GridData) * chi_sz, OBJ_TYPE__RAW_BYTES, NULL, 0);
			for (int k=0;k<chi_sz;k++) {
				Member *child = als_get(children, k);
				MddMemberRole *_mr_ = mdd_mr__create(child, mrole->dim_role);
				MddTuple *chi_tup = tuple_inset_mr(tup, _mr_);
				do_calculate_measure_value(md_ctx_, cube_, tuple__merge(ctx_tuple_, chi_tup), cellarr + k);
			}

			_bottop_sort_(bottop->type, cellarr, children);

			for (int k=0;k<chi_sz && k<count;k++) {
				MddTuple *chi_tup = tuple_inset_mr(tup, mdd_mr__create(als_get(children, k), mrole->dim_role));
				mddset__add_tuple(resset, chi_tup);
			}
			// }
		}
	} else if (bottop->uncertainexp && !bottop->sortexp) {
		LevelRole *lvrole = NULL;
		if (als_size(bottop->uncertainexp->plus_terms) == 1) {
			Term *term = als_get(bottop->uncertainexp->plus_terms, 0);
			if (als_size(term->mul_factories) == 1) {
				Factory *fac = als_get(term->mul_factories, 0);
				if (fac->t_cons == FACTORY_DEF__EU_PATH) {
					lvrole = up_evolving(md_ctx_, fac->up, cube_, ctx_tuple_);
					if (!lvrole || obj_type_of(lvrole) != OBJ_TYPE__LevelRole) {
						lvrole = NULL;
					}
				}
			}
		}

		if (lvrole) {
			
			for (int i=0;i<setsz;i++) {
				MddTuple *tup = als_get(set->tuples, i);
				mddset__add_tuple(resset, tup);

				MddMemberRole *mrole = NULL;
				for (int j=0;j<als_size(tup->mr_ls);j++) {
					mrole = als_get(tup->mr_ls, j);
					if (mrole->dim_role->gid == lvrole->dim_role->gid && mrole->member->lv == lvrole->lv->level) {
						break;
					} else {
						mrole = NULL;
					}
				}

				if (!mrole)
					continue;

				// if (mrole->dim_role->gid == lvrole->dim_role->gid && mrole->member->lv == lvrole->lv->level) {
				ArrayList *children = find_member_children(mrole->member);
				int chi_sz = als_size(children);
				GridData *cellarr = mam_alloc(sizeof(GridData) * chi_sz, OBJ_TYPE__RAW_BYTES, NULL, 0);
				for (int k=0;k<chi_sz;k++) {
					Member *child = als_get(children, k);
					MddMemberRole *_mr_ = mdd_mr__create(child, mrole->dim_role);
					MddTuple *chi_tup = tuple_inset_mr(tup, _mr_);
					do_calculate_measure_value(md_ctx_, cube_, tuple__merge(ctx_tuple_, chi_tup), cellarr + k);
				}

				_bottop_sort_(bottop->type, cellarr, children);

				for (int k=0;k<chi_sz && k<count;k++) {
					MddTuple *chi_tup = tuple_inset_mr(tup, mdd_mr__create(als_get(children, k), mrole->dim_role));
					mddset__add_tuple(resset, chi_tup);
				}
				// }
			}
	
		} else {
			int bottom_lval = 0;
			for (int i=0;i<setsz;i++) {
				MddTuple *tup = als_get(set->tuples, i);
				MddMemberRole *mr = als_get(tup->mr_ls, 0);
				if (mr->member->lv > bottom_lval)
					bottom_lval = mr->member->lv;
			}
			
			for (int i=0;i<setsz;i++) {
				MddTuple *tup = als_get(set->tuples, i);
				mddset__add_tuple(resset, tup);
				MddMemberRole *mrole = als_get(tup->mr_ls, 0);
				if (mrole->member->lv == bottom_lval) {
					ArrayList *children = find_member_children(mrole->member);
					int chi_sz = als_size(children);
					GridData *cellarr = mam_alloc(sizeof(GridData) * chi_sz, OBJ_TYPE__RAW_BYTES, NULL, 0);
					for (int k=0;k<chi_sz;k++) {
						Member *child = als_get(children, k);
						MddMemberRole *_mr_ = mdd_mr__create(child, mrole->dim_role);
						MddTuple *chi_tup = tuple_inset_mr(tup, _mr_);
						Expression_evaluate(md_ctx_, bottop->uncertainexp, cube_, tuple__merge(ctx_tuple_, chi_tup), cellarr + k);
					}

					_bottop_sort_(bottop->type, cellarr, children);

					for (int k=0;k<chi_sz && k<count;k++) {
						MddTuple *chi_tup = tuple_inset_mr(tup, mdd_mr__create(als_get(children, k), mrole->dim_role));
						mddset__add_tuple(resset, chi_tup);
					}
				}
			}
		}
	}

	return resset;
}

ArrayList *_ddm_fetch_dps_(MddSet *set) {
	ArrayList *mls = als_new(8, "<MddMemberRole *>", THREAD_MAM, NULL);
	for (int i=0;i<als_size(set->tuples);i++) {
		MddTuple *tup = als_get(set->tuples, i);
		MddMemberRole *mr = als_get(tup->mr_ls, 0);
		if (mr->dim_role->bin_attr & DR_MEASURE_MASK)
			continue;
		als_add(mls, mr);
	}

	int msz = als_size(mls);
	for (int i=0;i<msz-1;i++) {
		for (int j=i+1;j<msz;j++) {
			Member *mi = ((MddMemberRole *)als_get(mls, i))->member;
			Member *mj = ((MddMemberRole *)als_get(mls, j))->member;
			if (mj->lv < mi->lv) {
				void *vp = als_get(mls, i);
				ArrayList_set(mls, i, als_get(mls, j));
				ArrayList_set(mls, j, vp);
			}
		}
	}
	return mls;
}

// for ASTSetFunc_DrilldownMember
void *interpret_drilldownmember(void *md_ctx_, void *nil, void *ddm_, void *ctx_tuple_, void *cube_) {

	ASTSetFunc_DrilldownMember *drill = ddm_;

	MddSet *set1 = ids_setdef__build(md_ctx_, drill->setdef1, ctx_tuple_, cube_);
	MddSet *set2 = ids_setdef__build(md_ctx_, drill->setdef2, ctx_tuple_, cube_);

	ArrayList *mls = _ddm_fetch_dps_(set2);

	MddSet *resset = NULL;

	if (drill->recursive) {
		for (int i=0;i<als_size(mls);i++) {
			resset = mdd_set__create();
			MddMemberRole *mrole = als_get(mls, i);
			for (int j=0;j<als_size(set1->tuples);j++) {
				MddTuple *tuple = als_get(set1->tuples, j);
				mddset__add_tuple(resset, tuple);
				for (int k=0;k<als_size(tuple->mr_ls);k++) {
					MddMemberRole *mr = als_get(tuple->mr_ls, k);
					if (mr->dim_role->bin_attr & DR_MEASURE_MASK)
						continue;
					if (mr->dim_role->gid != mrole->dim_role->gid || mr->member->gid != mrole->member->gid)
						continue;

					ArrayList *children = find_member_children(mr->member);
					int chi_sz = als_size(children);
					for (int x=0;x<chi_sz;x++) {
						MddTuple *chi_tup = tuple_inset_mr(tuple, mdd_mr__create(als_get(children, x), mr->dim_role));
						mddset__add_tuple(resset, chi_tup);
					}
				}
			}
			// als_rm_index(mls, i--);
			set1 = resset;
		}
		return resset;
	}

	resset = mdd_set__create();
	for (int i=0;i<als_size(set1->tuples);i++) {
		MddTuple *tuple = als_get(set1->tuples, i);
		mddset__add_tuple(resset, tuple);
		for (int j=0;j<als_size(tuple->mr_ls);j++) {
			MddMemberRole *mr = als_get(tuple->mr_ls, j);
			if (mr->dim_role->bin_attr & DR_MEASURE_MASK)
				continue;
			for (int k=0;k<als_size(mls);k++) {
				MddMemberRole *mrole = als_get(mls, k);
				if (mr->dim_role->gid != mrole->dim_role->gid || mr->member->gid != mrole->member->gid)
					continue;

				ArrayList *children = find_member_children(mr->member);
				int chi_sz = als_size(children);
				for (int x=0;x<chi_sz;x++) {
					MddTuple *chi_tup = tuple_inset_mr(tuple, mdd_mr__create(als_get(children, x), mr->dim_role));
					mddset__add_tuple(resset, chi_tup);
				}
			}
		}
	}

	return resset;
}

// for ASTSetFunc_DrilldownMemberBottomTop
void *interpret_drilldownmemberbottomtop(void *md_ctx_, void *nil, void *ddmpt, void *ctx_tuple_, void *cube_) {
	ASTSetFunc_DrilldownMemberBottomTop *drill = ddmpt;

	MddSet *set1 = ids_setdef__build(md_ctx_, drill->setdef1, ctx_tuple_, cube_);
	MddSet *set2 = ids_setdef__build(md_ctx_, drill->setdef2, ctx_tuple_, cube_);

	ArrayList *s2mrs = _ddm_fetch_dps_(set2);

	GridData count_cel;
	Expression_evaluate(md_ctx_, drill->count_exp, cube_, ctx_tuple_, &count_cel);
	unsigned int count = (unsigned int)count_cel.val;

	MddSet *resset = NULL;

	if (drill->recursive) {

		for (int i=0; i < als_size(s2mrs) ;i++) {
			MddMemberRole *ssmrole = als_get(s2mrs, i);
			resset = mdd_set__create();
			for (int j=0; j < als_size(set1->tuples) ;j++) {
				MddTuple *stuple = als_get(set1->tuples, j);
				mddset__add_tuple(resset, stuple);
				for (int k=0; k<als_size(stuple->mr_ls) ;k++) {
					MddMemberRole *mr = als_get(stuple->mr_ls, k);
					if (mr->dim_role->bin_attr & DR_MEASURE_MASK)
						continue;
					if (mr->dim_role->gid != ssmrole->dim_role->gid || mr->member->gid != ssmrole->member->gid)
						continue;

					ArrayList *children = find_member_children(mr->member);
					GridData *cellarr = mam_alloc(sizeof(GridData) * als_size(children), OBJ_TYPE__RAW_BYTES, NULL, 0);

					for (int x=0;x<als_size(children);x++) {
						Member *child = als_get(children, x);
						MddTuple *chi_tup = tuple_inset_mr(stuple, mdd_mr__create(child, mr->dim_role));
						if (drill->num_exp) {
							Expression_evaluate(md_ctx_, drill->num_exp, cube_, tuple__merge(ctx_tuple_, chi_tup), cellarr + x);
						} else {
							do_calculate_measure_value(md_ctx_, cube_, tuple__merge(ctx_tuple_, chi_tup), cellarr + x);
						}
					}
					_bottop_sort_(drill->type, cellarr, children);

					for (int x=0;x<als_size(children) && x<count;x++) {
						MddTuple *chi_tup = tuple_inset_mr(stuple, mdd_mr__create(als_get(children, x), mr->dim_role));
						mddset__add_tuple(resset, chi_tup);
					}
				}
			}
			set1 = resset;
		}
		return resset;
	}

	resset = mdd_set__create();
	for (int i=0;i<als_size(set1->tuples);i++) {
		MddTuple *stuple = als_get(set1->tuples, i);
		mddset__add_tuple(resset, stuple);
		for (int j=0;j<als_size(stuple->mr_ls);j++) {
			MddMemberRole *mr = als_get(stuple->mr_ls, j);
			if (mr->dim_role->bin_attr & DR_MEASURE_MASK)
				continue;
			for (int k=0;k<als_size(s2mrs);k++) {
				MddMemberRole *ssmrole = als_get(s2mrs, k);
				if (mr->dim_role->gid != ssmrole->dim_role->gid || mr->member->gid != ssmrole->member->gid)
					continue;

				ArrayList *children = find_member_children(mr->member);
				GridData *cellarr = mam_alloc(sizeof(GridData) * als_size(children), OBJ_TYPE__RAW_BYTES, NULL, 0);

				for (int x=0;x<als_size(children);x++) {
					Member *child = als_get(children, x);
					MddTuple *chi_tup = tuple_inset_mr(stuple, mdd_mr__create(child, mr->dim_role));
					if (drill->num_exp) {
						Expression_evaluate(md_ctx_, drill->num_exp, cube_, tuple__merge(ctx_tuple_, chi_tup), cellarr + x);
					} else {
						do_calculate_measure_value(md_ctx_, cube_, tuple__merge(ctx_tuple_, chi_tup), cellarr + x);
					}
				}
				_bottop_sort_(drill->type, cellarr, children);

				int chi_sz = als_size(children);
				for (int x=0;x<chi_sz && x<count;x++) {
					MddTuple *chi_tup = tuple_inset_mr(stuple, mdd_mr__create(als_get(children, x), mr->dim_role));
					mddset__add_tuple(resset, chi_tup);
				}
			}
		}
	}

	return resset;
}

// for ASTSetFunc_DrillupLevel
void *interpret_drilluplevel(void *md_ctx_, void *nil, void *dul_, void *ctx_tuple_, void *cube_) {
	ASTSetFunc_DrillupLevel *drillup = dul_;
	MddSet *set = ids_setdef__build(md_ctx_, drillup->setdef, ctx_tuple_, cube_);

	LevelRole *lvrole = NULL;
	if (drillup->lrdef) {
		lvrole = up_evolving(md_ctx_, drillup->lrdef, cube_, ctx_tuple_);
		if (!lvrole || obj_type_of(lvrole) != OBJ_TYPE__LevelRole)
			lvrole = NULL;
	}

	int botlval = 0;
	if (!lvrole) {
		for (int i=0; i < als_size(set->tuples); i++) {
			MddTuple *tuple = als_get(set->tuples, i);
			MddMemberRole *mr = als_get(tuple->mr_ls, 0);
			if (i == 0 || (int)mr->member->lv > botlval)
				botlval = (int)mr->member->lv;
		}
	}

	for (int i = als_size(set->tuples) - 1; i >= 0; i--) {
		MddTuple *tuple = als_get(set->tuples, i);
		MddMemberRole *mr = als_get(tuple->mr_ls, 0);
		if (mr->member->lv >= (lvrole ? lvrole->lv->level : botlval)) {
			als_rm_index(set->tuples, i);
		}
	}

	return set;

}

// for ASTSetFunc_DrillupMember
void *interpret_drillupmember(void *md_ctx_, void *nil, void *dum_, void *ctx_tuple_, void *cube_) {
	ASTSetFunc_DrillupMember *drillupm = dum_;
	MddSet *set1 = ids_setdef__build(md_ctx_, drillupm->setdef1, ctx_tuple_, cube_);
	MddSet *set2 = ids_setdef__build(md_ctx_, drillupm->setdef2, ctx_tuple_, cube_);

	int s1_sz = als_size(set1->tuples);
	for (int i=als_size(set1->tuples)-1;i>=0;i--) {
		MddTuple *s1tup = als_get(set1->tuples, i);
		MddMemberRole *s1t_mr = als_get(s1tup->mr_ls, 0);
		for (int j=0;j<als_size(set2->tuples);j++) {
			MddTuple *s2tup = als_get(set2->tuples, j);
			MddMemberRole *s2t_mr = als_get(s2tup->mr_ls, 0);
			if (s1t_mr->member->p_gid == s2t_mr->member->gid) {
				als_rm_index(set1->tuples, i);
				break;
			}
		}		
	}

	return set1;
}

// for ASTSetFunc_Ancestors
void *interpret_Ancestors(void *md_ctx_, void *nil, void *ancestors_, void *ctx_tuple_, void *cube_) {

	ASTSetFunc_Ancestors *ancestors = ancestors_;

	MddMemberRole *mrole = up_evolving(md_ctx_, ancestors->mrdef, cube_, ctx_tuple_);
	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
		MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
		thrd_mam->exception_desc = "exception: interpret_Ancestors - The member cannot be determined.";
		longjmp(thrd_mam->excep_ctx_env, -1);
	}

	if (ancestors->lvdef) {
		LevelRole *lvrole = up_evolving(md_ctx_, ancestors->lvdef, cube_, ctx_tuple_);
		if (!lvrole || obj_type_of(lvrole) != OBJ_TYPE__LevelRole) {
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: interpret_Ancestors - The level cannot be determined.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
		ancestors->distance = mrole->member->lv - lvrole->lv->level;
	}

	if (ancestors->distance < 0 || mrole->member->lv - ancestors->distance < 0) {
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: interpret_Ancestors - Invalid distance.";
			longjmp(thrd_mam->excep_ctx_env, -1);
	}

	Member *m = mrole->member;
	for (int i=0;i<ancestors->distance;i++) {
		m = find_member_by_gid(m->p_gid);
	}

	MddTuple *tup = mdd_tp__create();
	mdd_tp__add_mbrole(tup, mdd_mr__create(m, mrole->dim_role));
	MddSet *set = mdd_set__create();
	mddset__add_tuple(set, tup);

	return set;
}

// for ASTSetFunc_BottomCount
void *interpret_BottomCount(void *md_ctx_, void *nil, void *bc, void *ctx_tuple_, void *cube_) {
	ASTSetFunc_BottomCount *bottomcount = bc;
	MddSet *set = ids_setdef__build(md_ctx_, bottomcount->setdef, ctx_tuple_, cube_);

	unsigned int setsz = als_size(set->tuples);
	GridData *cellarr = mam_alloc(sizeof(GridData) * setsz, OBJ_TYPE__RAW_BYTES, NULL, 0);
	for (int i=0;i<setsz;i++) {
		MddTuple *tup = als_get(set->tuples, i);
		if (bottomcount->exp) {
			Expression_evaluate(md_ctx_, bottomcount->exp, cube_, tuple__merge(ctx_tuple_, tup), cellarr + i);
		} else {
			do_calculate_measure_value(md_ctx_, cube_, tuple__merge(ctx_tuple_, tup), cellarr + i);
		}
	}

	GridData countcell;
	Expression_evaluate(md_ctx_, bottomcount->countexp, cube_, ctx_tuple_, &countcell);
	int count = (int)countcell.val;

	_bottop_sort_('b', cellarr, set->tuples);
	for (int i=setsz-1;i>=count;i--) {
		als_rm_index(set->tuples, i);
	}

	return set;
}

// for ASTSetFunc_BottomTopSum
void *interpret_BottomTopSum(void *md_ctx_, void *nil, void *bts, void *ctx_tuple_, void *cube_) {
	ASTSetFunc_BottomTopSum *botop = bts;
	MddSet *set = ids_setdef__build(md_ctx_, botop->setdef, ctx_tuple_, cube_);

	unsigned int setsz = als_size(set->tuples);
	GridData *cellarr = mam_alloc(sizeof(GridData) * setsz, OBJ_TYPE__RAW_BYTES, NULL, 0);
	for (int i=0;i<setsz;i++) {
		MddTuple *tup = als_get(set->tuples, i);
		Expression_evaluate(md_ctx_, botop->expdef2, cube_, tuple__merge(ctx_tuple_, tup), cellarr + i);
	}

	_bottop_sort_(botop->type, cellarr, set->tuples);

	GridData valcell;
	Expression_evaluate(md_ctx_, botop->expdef1, cube_, ctx_tuple_, &valcell);

	double sum = 0;
	for (int i=0;i<setsz;i++) {
		sum += cellarr[i].val;
		if (sum >= valcell.val) {
			for (int j=setsz-1;j>i;j--)
				als_rm_index(set->tuples, j);
			break;
		}
	}

	return set;
}

// for ASTSetFunc_Extract
void *interpret_Extract(void *md_ctx_, void *nil, void *extract_, void *ctx_tuple_, void *cube_) {
	ASTSetFunc_Extract *extract = extract_;
	MddSet *set = ids_setdef__build(md_ctx_, extract->setdef, ctx_tuple_, cube_);

	ArrayList *dimroles = als_new(512, "DimensionRole *", THREAD_MAM, NULL);
	ArrayList *heiroles = als_new(512, "HierarchyRole *", THREAD_MAM, NULL);

	unsigned int upsz = als_size(extract->dhlist);
	for (unsigned int i=0;i<upsz;i++) {
		void *ent = up_evolving(md_ctx_, als_get(extract->dhlist, i), cube_, ctx_tuple_);
		if (!ent)
			continue;
		if (obj_type_of(ent) == OBJ_TYPE__DimensionRole) {
			als_add(dimroles, ent);
		} if (obj_type_of(ent) == OBJ_TYPE__HierarchyRole) {
			als_add(heiroles, ent);
		}
	}

	if (als_size(dimroles) == 0 && als_size(heiroles) == 0) {
		MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
		thrd_mam->exception_desc = "exception: A exception throwed in fn:interpret_Extract.";
		longjmp(thrd_mam->excep_ctx_env, -1);
	}

	MddSet *result = mdd_set__create();

	unsigned int setsz = als_size(set->tuples);
	for (unsigned int i=0;i<setsz;i++) {
		MddTuple *tup = als_get(set->tuples, i);

		MddTuple *newtup = mdd_tp__create();

		for (unsigned int j=0;j<als_size(tup->mr_ls);j++) {
			MddMemberRole *mrole = als_get(tup->mr_ls, j);
			for (unsigned int d=0;d<als_size(dimroles);d++) {
				DimensionRole *dr = als_get(dimroles, d);
				if (mrole->dim_role->gid == dr->gid) {
					mdd_tp__add_mbrole(newtup, mrole);
					goto continue_j;
				}
			}
			for (unsigned int h=0;h<als_size(heiroles);h++) {
				HierarchyRole *hr = als_get(heiroles, h);
				if (mrole->dim_role->gid == hr->dim_role->gid && mrole->member->hierarchy_gid == hr->hierarchy->gid) {
					mdd_tp__add_mbrole(newtup, mrole);
					goto continue_j;
				}
			}
			continue_j:
		}

		mddset__add_tuple(result, newtup);
	}

	for (unsigned int i=als_size(result->tuples)-1;i>0;i--) {
		if (Tuple__cmp(als_get(result->tuples, i), als_get(result->tuples, i - 1)) == 0)
			als_rm_index(result->tuples, i);
	}

	return result;
}

// for ASTSetFunc_PeriodsToDate
void *interpret_PeriodsToDate(void *md_ctx_, void *nil, void *p2d, void *ctx_tuple_, void *cube_) {
	ASTSetFunc_PeriodsToDate *pertd = p2d;

	LevelRole *lvrole = NULL;
	MddMemberRole *mrole = NULL;

	MddTuple *ctx_tuple = ctx_tuple_;

	// PeriodsToDate()
	if (pertd->lrole_def == NULL && pertd->mrole_def == NULL) {
		ArrayList *datedrs = Cube_find_date_dim_roles(cube_);
		DimensionRole *date_dr = als_get(datedrs, 0);
		for (int i=0;i<als_size(ctx_tuple->mr_ls);i++) {
			MddMemberRole *mr = als_get(ctx_tuple->mr_ls, i);
			if (mr->dim_role->gid == date_dr->gid) {
				mrole = mr;
				break;
			}
		}
		for (int i=0;i<als_size(levels_pool);i++) {
			Level *lv = als_get(levels_pool, i);
			if (lv->hierarchy_gid == mrole->member->hierarchy_gid && lv->level == mrole->member->lv - 1) {
				lvrole = LevelRole_creat(lv, mrole->dim_role);
				break;
			}
		}
		goto fnlogical;
	}

	// PeriodsToDate(<level>)
	if (pertd->lrole_def && pertd->mrole_def == NULL) {
		lvrole = up_evolving(md_ctx_, pertd->lrole_def, cube_, ctx_tuple);
		for (int i=0;i<als_size(ctx_tuple->mr_ls);i++) {
			MddMemberRole *mr = als_get(ctx_tuple->mr_ls, i);
			if (mr->dim_role->gid == lvrole->dim_role->gid) {
				mrole = mr;
				break;
			}
		}	
		goto fnlogical;	
	}

	// PeriodsToDate(<level>, <member>)
	if (pertd->lrole_def && pertd->mrole_def) {
		lvrole = up_evolving(md_ctx_, pertd->lrole_def, cube_, ctx_tuple);
		mrole = up_evolving(md_ctx_, pertd->mrole_def, cube_, ctx_tuple);
	}

	fnlogical:

	MddSet *rsset = mdd_set__create();
	ArrayList *list = mdd__lv_ancestor_peer_descendants(lvrole->lv, mrole->member);
	for (int i=0;i<als_size(list);i++) {
		Member *mbr = als_get(list, i);
		MddTuple *tup = mdd_tp__create();
		mdd_tp__add_mbrole(tup, mdd_mr__create(mbr, mrole->dim_role));
		mddset__add_tuple(rsset, tup);
		if (mbr->gid == mrole->member->gid)
			break;
	}
	return rsset;
}