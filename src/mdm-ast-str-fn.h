#ifndef MDM_AST_STR_FN__H
#define MDM_AST_STR_FN__H 1

/*
 * for ASTStrFunc_Name
 *
 * @return <GridData *>
 */
void *strfn_name_interpreter(void *md_context, void *prefix_option, void *ast_member_func, void *context_tuple, void *cube);

/*
 * for ASTStrExp
 *
 * @return <GridData *>
 */
void *strexp_interpret(void *md_ctx_, void *pre_opt_, void *aststrexp_, void *ctx_tuple_, void *cube_);

#endif