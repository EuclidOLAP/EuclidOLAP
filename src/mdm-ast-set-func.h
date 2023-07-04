#ifndef MDM_AST_SET_FUNC__H
#define MDM_AST_SET_FUNC__H 1

#include "mdx-ast-struct.h"
#include "mdx.h"

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


typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *set_def;
    BooleanExpression *boolExp;
} ASTSetFunc_Filter;

// for ASTSetFunc_Filter
void *interpret_filter(void *md_ctx_, void *nil, void *filter_, void *ctx_tuple_, void *cube_);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mrole_up;
} ASTSetFunc_LateralMembers;

// for ASTSetFunc_LateralMembers
void *interpret_lateralmembers(void *md_ctx_, void *nil, void *lateral_, void *ctx_tuple_, void *cube_);


typedef enum fn_order_opt {
    ASC,
    DESC,
    BASC,
    BDESC
} FnOrderOpt;

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setsep;
    Expression *expsep;
    FnOrderOpt option;
} ASTSetFunc_Order;

// for ASTSetFunc_Order
void *interpret_order(void *md_ctx_, void *nil, void *order_, void *ctx_tuple_, void *cube_);


typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *set_def;
    Expression *count_exp;
    Expression *num_exp;
} ASTSetFunc_TopCount;

// for ASTSetFunc_TopCount
void *interpret_topcount(void *md_ctx_, void *setdef_, void *topcount_, void *ctx_tuple_, void *cube_);


typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef_1;
    SetDef *setdef_2;
    char all;
} ASTSetFunc_Except;

// for ASTSetFunc_Except
void *interpret_except(void *md_ctx_, void *nil, void *except_, void *ctx_tuple_, void *cube_);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mrole_def;
} ASTSetFunc_YTD;

// for ASTSetFunc_YTD
void *interpret_ytd(void *md_ctx_, void *mrole_, void *ytd_, void *ctx_tuple_, void *cube_);

#endif