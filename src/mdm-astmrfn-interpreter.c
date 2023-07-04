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

// void *interpret_ast_mrf_parent
//     (void *__md_context, void *__prefix_option, void *__ast_member_func, void *__context_tuple, void *__cube)
// {
//     log_print("[ debug ] interpret_ast_mrf_parent :: __md_context      %p\n", __md_context);
//     log_print("[ debug ] interpret_ast_mrf_parent :: __prefix_option   %p\n", __prefix_option);
//     log_print("[ debug ] interpret_ast_mrf_parent :: __ast_member_func %p\n", __ast_member_func);
//     log_print("[ debug ] interpret_ast_mrf_parent :: __context_tuple   %p\n", __context_tuple);
//     log_print("[ debug ] interpret_ast_mrf_parent :: __cube            %p\n", __cube);

//     MDContext *md_ctx = __md_context;
//     ASTMemberFunc_Parent *func_parent = __ast_member_func;
//     MddTuple *ctx_tuple = __context_tuple;
//     Cube *cube = __cube;

//     MddMemberRole *member_role = NULL;

//     if (__prefix_option)
//     {
//         short _type;
//         enum_oms _strat;
//         MemAllocMng *_mam;
//         obj_info(__prefix_option, &_type, &_strat, &_mam);

//         if (_type != OBJ_TYPE__MddMemberRole)
//         {
//             MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
//             thrd_mam->exception_desc = "exception: interpret_ast_mrf_parent :: __prefix_option is not a MddMemberRole.";
//             longjmp(thrd_mam->excep_ctx_env, -1);
//         }

//         member_role = __prefix_option;
//     }

//     if (member_role == NULL)
//         member_role = ids_mbrsdef__build(md_ctx, func_parent->ast_member, ctx_tuple, cube);

//     // when current member has no parent, return itself.
//     if (member_role->member->p_gid)
//         return mdd_mr__create(find_member_by_gid(member_role->member->p_gid), member_role->dim_role);
//     else
//         return member_role;
// }

// for ASTMemberFn_Parent
void *interpret_parent(void *md_ctx, void *mrole, void *parent, void *ctx_tuple, void *cube) {
    ASTMemberFn_Parent *func = parent;
    MddMemberRole *mr = mrole;

    if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
        if (!func->mr_up) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_parent.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }

        mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
        if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole) {
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
void *interpret_currentmember(void *md_ctx, void *drole, void *curmbr, void *ctx_tuple, void *cube) {

    ASTMemberFn_CurrentMember *func = curmbr;
    DimensionRole *dimrole = drole;

    if (!dimrole || obj_type_of(dimrole) != OBJ_TYPE__DimensionRole) {
        if (!func->dr_up) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_currentmember.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }

        dimrole = up_evolving(md_ctx, func->dr_up, cube, ctx_tuple);
        if (!dimrole || obj_type_of(dimrole) != OBJ_TYPE__DimensionRole) {
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
void *interpret_prevmember(void *md_ctx, void *mrole, void *prembr, void *ctx_tuple, void *cube) {
    ASTMemberFn_PrevMember *func = prembr;
    MddMemberRole *mr = mrole;

    if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
        if (!func->mr_up) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_prevmember.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }

        mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
        if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole) {
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
void *interpret_firstchild(void *md_ctx, void *mrole, void *firchi, void *ctx_tuple, void *cube) {
    ASTMemberFn_FirstChild *func = firchi;
    MddMemberRole *mr = mrole;

    if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
        if (!func->mr_up) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_firstchild.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }

        mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
        if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_firstchild.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }
    }

	Member *member = NULL;
	for (int i=0; i<als_size(member_pool); i++) {
		Member *m = als_get(member_pool, i);
		if (m->p_gid != mr->member->gid)
			continue;
		if (member == NULL || m->gid < member->gid)
			member = m;
	}
	return mdd_mr__create(member, mr->dim_role);
}

// for ASTMemberFn_LastChild
void *interpret_lastchild(void *md_ctx, void *mrole, void *laschi, void *ctx_tuple, void *cube) {
    ASTMemberFn_LastChild *func = laschi;
    MddMemberRole *mr = mrole;

    if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
        if (!func->mr_up) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_lastchild.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }

        mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
        if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_lastchild.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }
    }

	Member *member = NULL;
	for (int i=0; i<als_size(member_pool); i++) {
		Member *m = als_get(member_pool, i);
		if (m->p_gid != mr->member->gid)
			continue;
		if (member == NULL || m->gid > member->gid)
			member = m;
	}
	return mdd_mr__create(member, mr->dim_role);
}

// for ASTMemberFn_FirstSibling
void *interpret_firstsibling(void *md_ctx, void *mrole, void *firsib, void *ctx_tuple, void *cube) {
    ASTMemberFn_LastChild *func = firsib;
    MddMemberRole *mr = mrole;

    if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
        if (!func->mr_up) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_firstsibling.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }

        mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
        if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_firstsibling.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }
    }

	Member *member = NULL;
	for (int i=0; i<als_size(member_pool); i++) {
		Member *m = als_get(member_pool, i);
		if (m->p_gid != mr->member->p_gid)
			continue;
		if (member == NULL || m->gid <= member->gid)
			member = m;
	}
	return mdd_mr__create(member, mr->dim_role);
}