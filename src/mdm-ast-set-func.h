#ifndef MDM_AST_SET_FUNC__H
#define MDM_AST_SET_FUNC__H 1

#include "mdx-ast-struct.h"

typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mrole_sep;
} ASTSetFunc_Children;

// for ASTSetFunc_Children
void *interpret_children(void *md_ctx_, void *mrole_, void *ast_children_, void *ctx_tuple_, void *cube_);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *eup;
} ASTSetFunc_Members;

// for ASTSetFunc_Members
void *interpret_members(void *md_ctx_, void *entity_, void *ast_members_, void *ctx_tuple_, void *cube_);


typedef struct
{
    ASTFunctionCommonHead head;
    ArrayList *setdefs;
} ASTSetFunc_CrossJoin;

// for ASTSetFunc_CrossJoin
void *interpret_crossjoin(void *md_ctx_, void *nil, void *crossjoin_, void *ctx_tuple_, void *cube_);

#endif