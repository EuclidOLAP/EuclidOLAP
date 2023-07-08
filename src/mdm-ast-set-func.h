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

typedef enum fn_order_opt
{
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

typedef enum
{
    SELF,
    AFTER,
    BEFORE,
    BEFORE_AND_AFTER,
    SELF_AND_AFTER,
    SELF_AND_BEFORE,
    SELF_BEFORE_AFTER,
    LEAVES
} FnDescendantsOpt;

typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mrole_def;
    MDMEntityUniversalPath *lvrole_def;
    Expression *disexp;
    FnDescendantsOpt opt;
} ASTSetFunc_Descendants;

// for ASTSetFunc_Descendants
void *interpret_descendants(void *md_ctx_, void *nil, void *desc_, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *countexp;
} ASTSetFunc_Tail;

// for ASTSetFunc_Tail
void *interpret_tail(void *md_ctx_, void *nil, void *tail_, void *ctx_tuple_, void *cube_);

typedef enum
{
    BOTTOM_PER,
    TOP_PER
} FnBottomTopPercentOpt;

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *set;
    Expression *percentage;
    Expression *exp;
    FnBottomTopPercentOpt option;
} ASTSetFunc_BottomOrTopPercent;

// for ASTSetFunc_BottomOrTopPercent
void *interpret_bottomortoppercent(void *md_ctx_, void *nil, void *percent_, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    ArrayList *set_def_ls;
    char all_opt;
} ASTSetFunc_Union;

// for ASTSetFunc_Union
void *interpret_union(void *md_ctx_, void *nil, void *union_, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    ArrayList *set_def_ls;
    char all_opt;
} ASTSetFunc_Intersect;

// for ASTSetFunc_Intersect
void *interpret_intersect(void *md_ctx_, void *nil, void *intersect_, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
} ASTSetFunc_Distinct;

// for ASTSetFunc_Distinct
void *interpret_distinct(void *md_ctx_, void *set, void *dist, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    MDMEntityUniversalPath *lvrole_up;
    int index;
    char include_calc_members;
} ASTSetFunc_DrilldownLevel;

// for ASTSetFunc_DrilldownLevel
void *interpret_drilldownlevel(void *md_ctx_, void *nil, void *ddl, void *ctx_tuple_, void *cube_);


typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *countexp;
    Expression *uncertainexp;
    Expression *sortexp;
    char include_calc_members;
    char type; // 'b' - DrilldownLevelBottom, 't' - DrilldownLevelTop
} ASTSetFunc_DrilldownLevelBottomTop;

// for ASTSetFunc_DrilldownLevelBottomTop
void *interpret_drilldownlevelbottomtop(void *md_ctx_, void *nil, void *bottop_, void *ctx_tuple_, void *cube_);

#endif