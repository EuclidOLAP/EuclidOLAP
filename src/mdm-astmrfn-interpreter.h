#ifndef _MDM_ASTMRFN_INTERPRETER__H
#define _MDM_ASTMRFN_INTERPRETER__H 1

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

#endif