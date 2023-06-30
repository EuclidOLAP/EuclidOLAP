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
        for (int i=0; i<msz; i++) {
            Member *member = als_get(member_pool, i);

            if (member->dim_gid != dimrole->dim_gid)
                continue;

            MddTuple *tuple = mdd_tp__create();
            mdd_tp__add_mbrole(tuple, mdd_mr__create(member, dimrole));
            mddset__add_tuple(set, tuple);
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