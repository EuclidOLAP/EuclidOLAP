#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "mdx.h"
#include "mdd.h"
#include "mdm-astlogifn-interpreter.h"

// for ASTLogicalFunc_IsEmpty
void *interpret_ast_is_empty(void *md_context, void *grid_data, void *_is_empty, void *context_tuple, void *_cube) {

    MDContext *md_ctx = md_context;
    GridData *grid = grid_data;
    ASTLogicalFunc_IsEmpty *is_empty = _is_empty;
    MddTuple *ctx_tuple = context_tuple;
    Cube *cube = _cube;

    GridData exp_result;
    memset(&exp_result, 0, sizeof(GridData));

    Expression_evaluate(md_ctx, is_empty->exp, cube, ctx_tuple, &exp_result);

    grid->type = GRIDDATA_TYPE_BOOL;

    if ((exp_result.type == GRIDDATA_TYPE_NUM && exp_result.null_flag != 1)
        || (exp_result.type == GRIDDATA_TYPE_BOOL && exp_result.boolean)) {
        grid->boolean = 0;
    } else {
        grid->boolean = 1;
    }

    return NULL;
}