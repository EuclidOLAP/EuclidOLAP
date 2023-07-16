#ifndef _MDM_ASTLOGICALFN_INTERPRETER__H
#define _MDM_ASTLOGICALFN_INTERPRETER__H 1

#include "mdx-ast-struct.h"
#include "mdx.h"

typedef struct _ast_logical_func_is_empty_
{
    ASTFunctionCommonHead head;
    Expression *exp;
} ASTLogicalFunc_IsEmpty;

// for ASTLogicalFunc_IsEmpty
void *interpret_ast_is_empty(void *md_context, void *grid_data, void *is_empty, void *context_tuple, void *cube);

typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mrdef1;
    MDMEntityUniversalPath *mrdef2;
    char include_member;
} ASTLogicalFunc_IsAncestor;

// for ASTLogicalFunc_IsAncestor
void *interpret_IsAncestor(void *md_context, void *bool_cell, void *isancestor_, void *context_tuple, void *cube);

typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mrdef;
    int generation_number;
} ASTLogicalFunc_IsGeneration;

// for ASTLogicalFunc_IsGeneration
void *interpret_IsGeneration(void *md_context, void *bool_cell, void *isgeneration_, void *context_tuple, void *cube);

typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mrdef;
} ASTLogicalFunc_IsLeaf;

// for ASTLogicalFunc_IsLeaf
void *interpret_IsLeaf(void *md_context, void *bool_cell, void *isleaf_, void *context_tuple, void *cube);

typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mrdef1;
    MDMEntityUniversalPath *mrdef2;
    char include_member;
} ASTLogicalFunc_IsSibling;

// for ASTLogicalFunc_IsSibling
void *interpret_IsSibling(void *md_context, void *bool_cell, void *issibling_, void *context_tuple, void *cube);

#endif