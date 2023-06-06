#ifndef _MDM_ASTMRFN_INTERPRETER__H
#define _MDM_ASTMRFN_INTERPRETER__H 1

// for ASTMemberFunc_Parent
void * interpret_ast_mrf_parent
    (void *md_context, void *prefix_option, void *ast_member_func, void *context_tuple, void *cube);

#endif