#ifndef MDM_AST_NUM_FUNC__H
#define MDM_AST_NUM_FUNC__H 1

#include "mdx-ast-struct.h"
#include "mdx.h"

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef;
    char include_empty;
} ASTNumFunc_Avg;

// for ASTNumFunc_Avg
void *interpret_avg(void *md_ctx_, void *nil, void *avg_, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef;
    char opt; // 'x' - max; 'i' - min
} ASTNumFunc_MaxMin;

// for ASTNumFunc_MaxMin
void *interpret_maxmin(void *md_ctx_, void *nil, void *mm, void *ctx_tuple_, void *cube_);

typedef enum
{
    FAO_DEFAULT,
    FAO_SUM,
    FAO_COUNT,
    FAO_MAX,
    FAO_MIN,
    FAO_DISTINCT_COUNT
} FnAggregateOpt;

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef;
    FnAggregateOpt opt;
} ASTNumFunc_Aggregate;

// for ASTNumFunc_Aggregate
void *interpret_aggregate(void *md_ctx_, void *nil, void *agg, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef;
} ASTNumFunc_Sum;

// for ASTNumFunc_Sum
void *interpret_sum(void *md_ctx_, void *nil, void *sum_, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    char include_empty; // 0(def) - EXCLUDEEMPTY, 1 - INCLUDEEMPTY
} ASTNumFunc_Count;

// for ASTNumFunc_Count
void *interpret_count(void *md_ctx_, void *nil, void *count_, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef;
} ASTNumFunc_Median;

// for ASTNumFunc_Median
void *interpret_median(void *md_ctx_, void *nil, void *median_, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    void *param1; // a tuple_def or a up_def
    SetDef *setdef;
    Expression *expdef;
} ASTNumFunc_Rank;

// for ASTNumFunc_Rank
void *interpret_rank(void *md_ctx_, void *nil, void *rank_, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    Expression *expdef;
} ASTNumFunc_Abs;

// for ASTNumFunc_Abs
void *interpret_abs(void *md_ctx_, void *nil, void *abs_, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef_y;
    Expression *expdef_x;
} ASTNumFunc_Correlation;

// for ASTNumFunc_Correlation
void *interpret_correlation(void *md_ctx_, void *nil, void *cor, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef_y;
    Expression *expdef_x;
} ASTNumFunc_Covariance;

// for ASTNumFunc_Covariance
void *interpret_covariance(void *md_ctx_, void *nil, void *cov, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef_y;
    Expression *expdef_x;
} ASTNumFunc_LinRegIntercept;

// for ASTNumFunc_LinRegIntercept
void *interpret_LinRegIntercept(void *md_ctx_, void *nil, void *numfunc, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef_y;
    Expression *expdef_x;
} ASTNumFunc_LinRegSlope;

// for ASTNumFunc_LinRegSlope
void *interpret_LinRegSlope(void *md_ctx_, void *nil, void *numfunc, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef_y;
    Expression *expdef_x;
} ASTNumFunc_LinRegVariance;

// for ASTNumFunc_LinRegVariance
void *interpret_LinRegVariance(void *md_ctx_, void *nil, void *numfunc, void *ctx_tuple_, void *cube_);

typedef struct
{
    ASTFunctionCommonHead head;
    SetDef *setdef;
    Expression *expdef;
} ASTNumFunc_Stdev;

// for ASTNumFunc_Stdev
void *interpret_Stdev(void *md_ctx_, void *nil, void *numfunc, void *ctx_tuple_, void *cube_);

#endif