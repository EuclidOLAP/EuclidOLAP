#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "mdx.h"
#include "mdd.h"
#include "mdm-astlogifn-interpreter.h"

extern ArrayList *dims_pool;
extern ArrayList *hierarchies_pool;
extern ArrayList *member_pool;
extern ArrayList *cubes_pool;
extern ArrayList *levels_pool;

// for ASTLogicalFunc_IsEmpty
void *interpret_ast_is_empty(void *md_context, void *grid_data, void *_is_empty, void *context_tuple, void *_cube)
{

    MDContext *md_ctx = md_context;
    GridData *grid = grid_data;
    ASTLogicalFunc_IsEmpty *is_empty = _is_empty;
    MddTuple *ctx_tuple = context_tuple;
    Cube *cube = _cube;

    GridData exp_result;
    memset(&exp_result, 0, sizeof(GridData));

    Expression_evaluate(md_ctx, is_empty->exp, cube, ctx_tuple, &exp_result);

    grid->type = GRIDDATA_TYPE_BOOL;

    if ((exp_result.type == GRIDDATA_TYPE_NUM && exp_result.null_flag != 1) || (exp_result.type == GRIDDATA_TYPE_BOOL && exp_result.boolean))
    {
        grid->boolean = 0;
    }
    else
    {
        grid->boolean = 1;
    }

    return NULL;
}

// for ASTLogicalFunc_IsAncestor
void *interpret_IsAncestor(void *md_context, void *bool_cell, void *isancestor_, void *context_tuple, void *cube) {
    ASTLogicalFunc_IsAncestor *isancestor = isancestor_;

    MddMemberRole *mrole1 = up_evolving(md_context, isancestor->mrdef1, cube, context_tuple);
    MddMemberRole *mrole2 = up_evolving(md_context, isancestor->mrdef2, cube, context_tuple);

    // GridData *cell = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);
    GridData *cell = bool_cell;
    cell->type = GRIDDATA_TYPE_BOOL;
    cell->boolean = GRIDDATA_BOOL_FALSE;
    if (!mrole1 || !mrole2 || obj_type_of(mrole1) != OBJ_TYPE__MddMemberRole || obj_type_of(mrole2) != OBJ_TYPE__MddMemberRole) {
        return cell;
    }

    Member *m1 = mrole1->member;
    Member *m2 = mrole2->member;

    if (isancestor->include_member && m1->gid == m2->gid) {
        cell->boolean = GRIDDATA_BOOL_TRUE;
        return cell;
    }

    while (1)
    {
        m2 = find_member_by_gid(m2->p_gid);
        if (!m2)
            break;
        if (m2->gid == m1->gid) {
            cell->boolean = GRIDDATA_BOOL_TRUE;
            break;
        }
    }

    return cell;
}

// for ASTLogicalFunc_IsGeneration
void *interpret_IsGeneration(void *md_context, void *bool_cell, void *isgeneration_, void *context_tuple, void *cube) {
    ASTLogicalFunc_IsGeneration *isgeneration = isgeneration_;

    GridData *cell = bool_cell;
    cell->type = GRIDDATA_TYPE_BOOL;
    cell->boolean = GRIDDATA_BOOL_FALSE;

    MddMemberRole *mrole = up_evolving(md_context, isgeneration->mrdef, cube, context_tuple);
    if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
        return cell;
    }

    ArrayList *hyls = als_new(8, "Hierarchy *", THREAD_MAM, NULL);

    for (int i=0;i<als_size(hierarchies_pool);i++) {
        Hierarchy *hy = als_get(hierarchies_pool, i);
        if (hy->dimension_gid == mrole->dim_role->dim_gid)
            als_add(hyls, hy);
    }

    if (isgeneration->generation_number > als_size(hyls))
        return cell;

    Hierarchy *hy = als_get(hyls, isgeneration->generation_number - 1);
    if (mrole->member->hierarchy_gid == hy->gid)
        cell->boolean = GRIDDATA_BOOL_TRUE;

    return cell;
}

// for ASTLogicalFunc_IsLeaf
void *interpret_IsLeaf(void *md_context, void *bool_cell, void *isleaf_, void *context_tuple, void *cube) {
    ASTLogicalFunc_IsLeaf *isleaf = isleaf_;
    MddMemberRole *mrole = up_evolving(md_context, isleaf->mrdef, cube, context_tuple);
    GridData *cell = bool_cell;
    cell->type = GRIDDATA_TYPE_BOOL;
    cell->boolean = GRIDDATA_BOOL_FALSE;
    if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
        return cell;
    }
    if (!(mrole->member->bin_attr & MDD_MEMBER__BIN_ATTR_FLAG__NON_LEAF))
        cell->boolean = GRIDDATA_BOOL_TRUE;
    return cell;
}