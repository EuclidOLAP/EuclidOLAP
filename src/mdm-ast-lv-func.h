#ifndef MDM_AST_LEVEL_FUNC__H
#define MDM_AST_LEVEL_FUNC__H 1

#include "mdx-ast-struct.h"
#include "mdx.h"

typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mrdef;
} ASTLvFunc_Level;

// for ASTLvFunc_Level
void *interpret_Level(void *md_ctx_, void *mr, void *lv_fn, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *dhdef;
    Expression *lvexp;
} ASTLvFunc_Levels;

// for ASTLvFunc_Levels
void *interpret_Levels(void *md_ctx_, void *dhr, void *lvs_fn, void *ctx_tuple_, void *cube_);

#endif