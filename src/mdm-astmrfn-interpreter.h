#ifndef _MDM_ASTMRFN_INTERPRETER__H
#define _MDM_ASTMRFN_INTERPRETER__H 1

#include "mdx-ast-struct.h"

// // for ASTMemberFunc_Parent
// void * interpret_ast_mrf_parent
//     (void *md_context, void *prefix_option, void *ast_member_func, void *context_tuple, void *cube);

typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mr_up;
} ASTMemberFn_Parent;

// for ASTMemberFn_Parent
void *interpret_parent(void *md_ctx, void *mrole, void *parent, void *ctx_tuple, void *cube);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *dr_up;
} ASTMemberFn_CurrentMember;

// for ASTMemberFn_CurrentMember
void *interpret_currentmember(void *md_ctx, void *drole, void *curmbr, void *ctx_tuple, void *cube);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mr_up;
} ASTMemberFn_PrevMember;

// for ASTMemberFn_PrevMember
void *interpret_prevmember(void *md_ctx, void *mrole, void *prembr, void *ctx_tuple, void *cube);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mr_up;
} ASTMemberFn_FirstChild;

// for ASTMemberFn_FirstChild
void *interpret_firstchild(void *md_ctx, void *mrole, void *firchi, void *ctx_tuple, void *cube);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mr_up;
} ASTMemberFn_LastChild;

// for ASTMemberFn_LastChild
void *interpret_lastchild(void *md_ctx, void *mrole, void *laschi, void *ctx_tuple, void *cube);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mr_up;
} ASTMemberFn_FirstSibling;

// for ASTMemberFn_FirstSibling
void *interpret_firstsibling(void *md_ctx, void *mrole, void *firsib, void *ctx_tuple, void *cube);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mr_up;
} ASTMemberFn_LastSibling;

// for ASTMemberFn_LastSibling
void *interpret_lastsibling(void *md_ctx, void *mrole, void *lassib, void *ctx_tuple, void *cube);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *mr_up;
    long index;
} ASTMemberFn_Lag;

// for ASTMemberFn_Lag
void *interpret_lag(void *md_ctx, void *mrole, void *lag, void *ctx_tuple, void *cube);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *lvroleup;
    Expression *index;
    MDMEntityUniversalPath *mroleup;
} ASTMemberFn_ParallelPeriod;

// for ASTMemberFn_ParallelPeriod
void *interpret_parallelperiod(void *md_ctx, void *nil, void *pp, void *ctx_tuple, void *cube);


typedef struct
{
    ASTFunctionCommonHead head;
    MDMEntityUniversalPath *lvroleup;
    MDMEntityUniversalPath *mroleup;
} ASTMemberFn_ClosingPeriod;

// for ASTMemberFn_ClosingPeriod
void *interpret_closingperiod(void *md_ctx, void *nil, void *cp, void *ctx_tuple, void *cube);

#endif