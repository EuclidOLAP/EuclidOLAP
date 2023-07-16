#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "mdx.h"
#include "mdd.h"
#include "mdm-astmrfn-interpreter.h"

extern ArrayList *dims_pool;
extern ArrayList *hierarchies_pool;
extern ArrayList *member_pool;
extern ArrayList *cubes_pool;
extern ArrayList *levels_pool;

// for ASTMemberFn_Parent
void *interpret_parent(void *md_ctx, void *mrole, void *parent, void *ctx_tuple, void *cube)
{
	ASTMemberFn_Parent *func = parent;
	MddMemberRole *mr = mrole;

	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
	{
		if (!func->mr_up)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_parent.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
		if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_parent.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	// when current member has no parent, return itself.
	if (mr->member->p_gid)
		return mdd_mr__create(find_member_by_gid(mr->member->p_gid), mr->dim_role);
	else
		return mr;
}

// for ASTMemberFn_CurrentMember
void *interpret_currentmember(void *md_ctx, void *drole, void *curmbr, void *ctx_tuple, void *cube)
{

	ASTMemberFn_CurrentMember *func = curmbr;
	DimensionRole *dimrole = drole;

	if (!dimrole || obj_type_of(dimrole) != OBJ_TYPE__DimensionRole)
	{
		if (!func->dr_up)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_currentmember.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		dimrole = up_evolving(md_ctx, func->dr_up, cube, ctx_tuple);
		if (!dimrole || obj_type_of(dimrole) != OBJ_TYPE__DimensionRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_currentmember.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	MddTuple *context_tuple = ctx_tuple;
	int mrs_count = als_size(context_tuple->mr_ls);

	for (int i = 0; i < mrs_count; i++)
	{
		MddMemberRole *mbrRole = als_get(context_tuple->mr_ls, i);
		if (dimrole->gid == mbrRole->dim_role->gid)
			return mbrRole;
	}

	log_print("[ error ] - ASTMemberFn_CurrentMember do not matching DimensionRole - < %s >\n", dimrole->name);
	return NULL;
}

// for ASTMemberFn_PrevMember
void *interpret_prevmember(void *md_ctx, void *mrole, void *prembr, void *ctx_tuple, void *cube)
{
	ASTMemberFn_PrevMember *func = prembr;
	MddMemberRole *mr = mrole;

	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
	{
		if (!func->mr_up)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_prevmember.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
		if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_prevmember.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	int i, len;
	Member *prev = NULL;

	if (mr->dim_role->gid)
	{
		// measure
		len = als_size(((Cube *)cube)->measure_mbrs);
		if (len < 2)
			return mr;

		for (i = 0; i < len; i++)
		{
			Member *mea_m = als_get(((Cube *)cube)->measure_mbrs, i);
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

// for ASTMemberFn_FirstChild
void *interpret_firstchild(void *md_ctx, void *mrole, void *firchi, void *ctx_tuple, void *cube)
{
	ASTMemberFn_FirstChild *func = firchi;
	MddMemberRole *mr = mrole;

	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
	{
		if (!func->mr_up)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_firstchild.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
		if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_firstchild.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	Member *member = NULL;
	for (int i = 0; i < als_size(member_pool); i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->p_gid != mr->member->gid)
			continue;
		if (member == NULL || m->gid < member->gid)
			member = m;
	}
	return mdd_mr__create(member, mr->dim_role);
}

// for ASTMemberFn_LastChild
void *interpret_lastchild(void *md_ctx, void *mrole, void *laschi, void *ctx_tuple, void *cube)
{
	ASTMemberFn_LastChild *func = laschi;
	MddMemberRole *mr = mrole;

	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
	{
		if (!func->mr_up)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_lastchild.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
		if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_lastchild.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	Member *member = NULL;
	for (int i = 0; i < als_size(member_pool); i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->p_gid != mr->member->gid)
			continue;
		if (member == NULL || m->gid > member->gid)
			member = m;
	}
	return mdd_mr__create(member, mr->dim_role);
}

// for ASTMemberFn_FirstSibling
void *interpret_firstsibling(void *md_ctx, void *mrole, void *firsib, void *ctx_tuple, void *cube)
{
	ASTMemberFn_FirstSibling *func = firsib;
	MddMemberRole *mr = mrole;

	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
	{
		if (!func->mr_up)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_firstsibling.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
		if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_firstsibling.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	Member *member = NULL;
	for (int i = 0; i < als_size(member_pool); i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->p_gid != mr->member->p_gid)
			continue;
		if (member == NULL || m->gid <= member->gid)
			member = m;
	}
	return mdd_mr__create(member, mr->dim_role);
}

// for ASTMemberFn_LastSibling
void *interpret_lastsibling(void *md_ctx, void *mrole, void *lassib, void *ctx_tuple, void *cube)
{
	ASTMemberFn_LastSibling *func = lassib;
	MddMemberRole *mr = mrole;

	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
	{
		if (!func->mr_up)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_lastsibling.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
		if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_lastsibling.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	Member *member = NULL;
	for (int i = 0; i < als_size(member_pool); i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->p_gid != mr->member->p_gid)
			continue;
		if (member == NULL || m->gid >= member->gid)
			member = m;
	}
	return mdd_mr__create(member, mr->dim_role);
}

int __mr_fn_lag_cmp__(void *obj, void *other)
{
	Member *mobj = (Member *)obj;
	Member *moth = (Member *)other;
	return moth->gid < mobj->gid ? -1 : (moth->gid > mobj->gid ? 1 : 0);
}

// for ASTMemberFn_Lag
void *interpret_lag(void *md_ctx, void *mrole, void *lag, void *ctx_tuple, void *cube)
{
	ASTMemberFn_Lag *func = lag;
	MddMemberRole *mr = mrole;

	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
	{
		if (!func->mr_up)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_lag.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
		if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_lag.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	if (func->index == 0)
		return mr;

	ArrayList *list = als_new(64, "Member *", THREAD_MAM, NULL);

	for (int i = 0; i < als_size(member_pool); i++)
	{
		Member *m = als_get(member_pool, i);
		if (m->p_gid == mr->member->p_gid)
			als_add(list, m);
	}

	ArrayList_sort(list, __mr_fn_lag_cmp__);

	int m_idx = 0;
	for (int i = 0; i < als_size(list); i++)
	{
		Member *m = als_get(list, i);
		if (m->gid == mr->member->gid)
		{
			m_idx = i;
			break;
		}
	}

	m_idx -= func->index;

	if (m_idx < 0)
	{
		m_idx = 0;
	}
	else if (m_idx >= als_size(list))
	{
		m_idx = als_size(list) - 1;
	}

	return mdd_mr__create(als_get(list, m_idx), mr->dim_role);
}

// for ASTMemberFn_ParallelPeriod
void *interpret_parallelperiod(void *md_ctx, void *nil, void *pp, void *ctx_tuple, void *cube)
{

	ASTMemberFn_ParallelPeriod *paper = pp;
	MddTuple *context_tuple = ctx_tuple;

	// ParallelPeriod()
	if (paper->lvroleup == NULL)
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
	if (paper->index == NULL)
	{
		LevelRole *lv_role = up_evolving(md_ctx, paper->lvroleup, cube, context_tuple);
		if (!lv_role || obj_type_of(lv_role) != OBJ_TYPE__LevelRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_parallelperiod.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

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
	if (paper->mroleup == NULL)
	{
		LevelRole *lv_role = up_evolving(md_ctx, paper->lvroleup, cube, context_tuple);
		if (!lv_role || obj_type_of(lv_role) != OBJ_TYPE__LevelRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_parallelperiod.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

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
		Expression_evaluate(md_ctx, paper->index, cube, context_tuple, &prev_offset);

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
	LevelRole *lv_role = up_evolving(md_ctx, paper->lvroleup, cube, context_tuple);
	if (!lv_role || obj_type_of(lv_role) != OBJ_TYPE__LevelRole)
	{
		MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
		thrd_mam->exception_desc = "exception: function: interpret_parallelperiod.";
		longjmp(thrd_mam->excep_ctx_env, -1);
	}

	MddMemberRole *mr = up_evolving(md_ctx, paper->mroleup, cube, context_tuple);
	if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole)
	{
		MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
		thrd_mam->exception_desc = "exception: function: interpret_parallelperiod.";
		longjmp(thrd_mam->excep_ctx_env, -1);
	}

	if (mr->member->lv < lv_role->lv->level)
		return NULL;

	GridData prev_offset;
	Expression_evaluate(md_ctx, paper->index, cube, context_tuple, &prev_offset);
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

// for ASTMemberFn_ClosingPeriod
void *interpret_closingperiod(void *md_ctx, void *nil, void *cp, void *ctx_tuple, void *cube)
{

	ASTMemberFn_ClosingPeriod *cloper = cp;

	if (cloper->lvroleup == NULL && cloper->mroleup == NULL)
	{
		ArrayList *roles_of_date_dims = Cube_find_date_dim_roles(cube);
		if (als_size(roles_of_date_dims) != 1)
			return NULL;

		DimensionRole *date_dim_role = als_get(roles_of_date_dims, 0);
		Level *level = NULL;
		Level *lv = NULL;
		for (int i = 0; i < als_size(levels_pool); i++)
		{
			lv = als_get(levels_pool, i);
			if (lv->dim_gid != date_dim_role->dim_gid || lv->level < 1)
				continue;

			if (level == NULL)
				level = lv;
			else if (lv->level < level->level)
				level = lv;
		}

		Member *member = NULL;
		for (int i = 0; i < als_size(member_pool); i++)
		{
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

	if (cloper->lvroleup != NULL && cloper->mroleup == NULL)
	{
		LevelRole *lv_role = up_evolving(md_ctx, cloper->lvroleup, cube, ctx_tuple);
		if (!lv_role || obj_type_of(lv_role) != OBJ_TYPE__LevelRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_closingperiod.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		Member *member = NULL;
		for (int i = 0; i < als_size(member_pool); i++)
		{
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

	if (cloper->lvroleup != NULL && cloper->mroleup != NULL)
	{
		LevelRole *lv_role = up_evolving(md_ctx, cloper->lvroleup, cube, ctx_tuple);
		if (!lv_role || obj_type_of(lv_role) != OBJ_TYPE__LevelRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_closingperiod.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		MddMemberRole *m_role = up_evolving(md_ctx, cloper->mroleup, cube, ctx_tuple);
		if (!m_role || obj_type_of(m_role) != OBJ_TYPE__MddMemberRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_closingperiod.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		ArrayList *descendants = Member__descendants(m_role->member);
		Member *member = NULL;
		for (int i = 0; i < als_size(descendants); i++)
		{
			Member *m = als_get(descendants, i);
			if (m->lv != lv_role->lv->level)
				continue;
			if (member == NULL || m->gid > member->gid)
				member = m;
		}
		return mdd_mr__create(member, lv_role->dim_role);
	}

	log_print("[ error ] interpret_closingperiod\n");
	exit(EXIT_FAILURE);
}

// for ASTMemberFn_OpeningPeriod
void *interpret_openingperiod(void *md_ctx, void *nil, void *op, void *ctx_tuple, void *cube)
{

	ASTMemberFn_OpeningPeriod *opeper = op;

	if (opeper->lvroleup == NULL && opeper->mroleup == NULL)
	{
		ArrayList *roles_of_date_dims = Cube_find_date_dim_roles(cube);
		if (als_size(roles_of_date_dims) != 1)
			return NULL;

		DimensionRole *date_dim_role = als_get(roles_of_date_dims, 0);
		Level *level = NULL;
		Level *lv = NULL;
		for (int i = 0; i < als_size(levels_pool); i++)
		{
			lv = als_get(levels_pool, i);
			if (lv->dim_gid != date_dim_role->dim_gid || lv->level < 1)
				continue;

			if (level == NULL)
				level = lv;
			else if (lv->level < level->level)
				level = lv;
		}

		Member *member = NULL;
		for (int i = 0; i < als_size(member_pool); i++)
		{
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

	if (opeper->lvroleup != NULL && opeper->mroleup == NULL)
	{
		LevelRole *lv_role = up_evolving(md_ctx, opeper->lvroleup, cube, ctx_tuple);
		if (!lv_role || obj_type_of(lv_role) != OBJ_TYPE__LevelRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_openingperiod.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		Member *member = NULL;
		for (int i = 0; i < als_size(member_pool); i++)
		{
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

	if (opeper->lvroleup != NULL && opeper->mroleup != NULL)
	{
		LevelRole *lv_role = up_evolving(md_ctx, opeper->lvroleup, cube, ctx_tuple);
		if (!lv_role || obj_type_of(lv_role) != OBJ_TYPE__LevelRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_openingperiod.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		MddMemberRole *m_role = up_evolving(md_ctx, opeper->mroleup, cube, ctx_tuple);
		if (!m_role || obj_type_of(m_role) != OBJ_TYPE__MddMemberRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_openingperiod.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		ArrayList *descendants = Member__descendants(m_role->member);
		Member *member = NULL;
		for (int i = 0; i < als_size(descendants); i++)
		{
			Member *m = als_get(descendants, i);
			if (m->lv != lv_role->lv->level)
				continue;
			if (member == NULL || m->gid < member->gid)
				member = m;
		}
		return mdd_mr__create(member, lv_role->dim_role);
	}

	log_print("[ error ] interpret_openingperiod\n");
	exit(EXIT_FAILURE);
}

// for ASTMemberFn_NextMember
void *interpret_nextmember(void *md_ctx, void *mrole_, void *nm, void *ctx_tuple, void *cube_)
{

	ASTMemberFn_NextMember *nexmem = nm;
	MddMemberRole *mrole = mrole_;

	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
	{
		if (!nexmem->mroleup)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_nextmember.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}

		mrole = up_evolving(md_ctx, nexmem->mroleup, cube_, ctx_tuple);
		if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole)
		{
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: function: interpret_nextmember.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
	}

	Cube *cube = cube_;
	Member *meam = NULL;

	if (mrole->dim_role->bin_attr & DR_MEASURE_MASK)
	{
		// mrole is a measure member role

		for (int i = 0; i < als_size(cube->measure_mbrs); i++)
		{
			Member *m = als_get(cube->measure_mbrs, i);
			if (m->gid <= mrole->member->gid)
				continue;
			if (meam == NULL || m->gid < meam->gid)
				meam = m;
		}
	}
	else
	{
		// mrole is a non measure member role

		unsigned int mpsz = als_size(member_pool);
		for (unsigned int i = 0; i < mpsz; i++)
		{
			Member *m = als_get(member_pool, i);
			if (m->lv != mrole->member->lv)
				continue;

			if (compare_member_position(mrole->member, m) < 1)
				continue;

			if (meam == NULL || compare_member_position(meam, m) < 0)
				meam = m;
		}
	}
	return meam ? mdd_mr__create(meam, mrole->dim_role) : mrole;
}

// for ASTMemberFn_Ancestor
void *interpret_Ancestor(void *md_ctx_, void *mr, void *anc, void *ctx_tuple_, void *cube_) {

	ASTMemberFn_Ancestor *ancestor = anc;

	MddMemberRole *mrole = mr;
	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
		mrole = up_evolving(md_ctx_, ancestor->mrdef, cube_, ctx_tuple_);
		if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: interpret_Ancestor - The member cannot be determined.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}		
	}

	if (ancestor->lvdef) {
		LevelRole *lvrole = up_evolving(md_ctx_, ancestor->lvdef, cube_, ctx_tuple_);
		if (!lvrole || obj_type_of(lvrole) != OBJ_TYPE__LevelRole) {
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: interpret_Ancestor - The level cannot be determined.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}
		ancestor->distance = mrole->member->lv - lvrole->lv->level;
	}

	if (ancestor->distance < 0 || mrole->member->lv - ancestor->distance < 0) {
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: interpret_Ancestor - Invalid distance.";
			longjmp(thrd_mam->excep_ctx_env, -1);
	}

	Member *m = mrole->member;
	for (int i=0;i<ancestor->distance;i++) {
		m = find_member_by_gid(m->p_gid);
	}

	return mdd_mr__create(m, mrole->dim_role);
}

// for ASTMemberFn_Cousin
void *interpret_Cousin(void *md_ctx_, void *mr, void *cousin_, void *ctx_tuple_, void *cube_) {
	ASTMemberFn_Cousin *cousin = cousin_;

	MddMemberRole *mrole = mr;
	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
		mrole = up_evolving(md_ctx_, cousin->mrdef, cube_, ctx_tuple_);
		if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
			MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
			thrd_mam->exception_desc = "exception: interpret_Cousin - The member cannot be determined.";
			longjmp(thrd_mam->excep_ctx_env, -1);
		}		
	}

	MddMemberRole *ance = up_evolving(md_ctx_, cousin->ancedef, cube_, ctx_tuple_);
	if (!ance || obj_type_of(ance) != OBJ_TYPE__MddMemberRole) {
		MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
		thrd_mam->exception_desc = "exception: interpret_Cousin - The ance cannot be determined.";
		longjmp(thrd_mam->excep_ctx_env, -1);
	}

	ArrayList *siblings = find_member_children(find_member_by_gid(mrole->member->p_gid));
	int idx = -1;
	for (int i=0;i<als_size(siblings);i++) {
		Member *sibling = als_get(siblings, i);
		if (sibling->gid == mrole->member->gid) {
			idx = i;
			break;
		}
	}

	siblings = find_member_children(ance->member);

	if (idx < als_size(siblings)) {
		return mdd_mr__create(als_get(siblings, idx), ance->dim_role);
	}

	return NULL;
}