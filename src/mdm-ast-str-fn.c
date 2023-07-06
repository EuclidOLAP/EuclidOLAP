#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "mdx.h"
#include "mdd.h"
#include "mdm-ast-str-fn.h"

// for ASTStrFunc_Name
// return <GridData *>
void *strfn_name_interpreter(void *_md_ctx, void *_entity, void *_func, void *_ctx_tuple, void *_cube)
{

    MDContext *md_ctx = _md_ctx;
    void *entity = _entity;
    ASTStrFunc_Name *func = _func;
    MddTuple *ctx_tuple = _ctx_tuple;
    Cube *cube = _cube;

    if (entity && func->up)
    {
        MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
        thrd_mam->exception_desc = "exception: strfn_name_interpreter: Duplicate parameters.";
        longjmp(thrd_mam->excep_ctx_env, -1);
    }

    if (!entity)
        entity = up_evolving(md_ctx, func->up, cube, ctx_tuple);

    GridData *gd = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);
    gd->type = GRIDDATA_TYPE_STR;

    switch (obj_type_of(entity))
    {
    case OBJ_TYPE__DimensionRole:
        gd->str = ((DimensionRole *)entity)->name;
        break;
    case OBJ_TYPE__HierarchyRole:
        gd->str = ((HierarchyRole *)entity)->hierarchy->name;
        break;
    case OBJ_TYPE__LevelRole:
        gd->str = ((LevelRole *)entity)->lv->name;
        break;
    case OBJ_TYPE__MddMemberRole:
        MddMemberRole *mr = entity;
        if (mr->member)
            gd->str = mr->member->name;
        else
            gd->str = "#Formula Member#";
        break;
    default:
        gd->str = "???";
        break;
    }

    return gd;
}

// for ASTStrExp
// return <GridData *>
void *strexp_interpret(void *md_ctx_, void *pre_opt_, void *aststrexp_, void *ctx_tuple_, void *cube_)
{

    MDContext *md_ctx = md_ctx_;
    ASTStrExp *strexp = aststrexp_;
    MddTuple *ctx_tuple = ctx_tuple_;
    Cube *cube = cube_;

    if (strexp->type == STR_LITERAL)
    {
        GridData *gd = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);
        gd->type = GRIDDATA_TYPE_STR;
        gd->str = strexp->part.str;
        return gd;
    }
    else if (strexp->type == STR_FUNC)
    {
        return ((ASTFunctionCommonHead *)strexp->part.str_func)->interpret(md_ctx, pre_opt_, strexp->part.str_func, ctx_tuple, cube);
    }

    log_print("[ Fatal ] strexp_interpret ::\n");
    exit(EXIT_FAILURE);
}