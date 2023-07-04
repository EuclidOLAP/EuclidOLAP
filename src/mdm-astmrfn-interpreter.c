#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "mdx.h"
#include "mdd.h"
#include "mdm-astmrfn-interpreter.h"

// void *interpret_ast_mrf_parent
//     (void *__md_context, void *__prefix_option, void *__ast_member_func, void *__context_tuple, void *__cube)
// {
//     log_print("[ debug ] interpret_ast_mrf_parent :: __md_context      %p\n", __md_context);
//     log_print("[ debug ] interpret_ast_mrf_parent :: __prefix_option   %p\n", __prefix_option);
//     log_print("[ debug ] interpret_ast_mrf_parent :: __ast_member_func %p\n", __ast_member_func);
//     log_print("[ debug ] interpret_ast_mrf_parent :: __context_tuple   %p\n", __context_tuple);
//     log_print("[ debug ] interpret_ast_mrf_parent :: __cube            %p\n", __cube);

//     MDContext *md_ctx = __md_context;
//     ASTMemberFunc_Parent *func_parent = __ast_member_func;
//     MddTuple *ctx_tuple = __context_tuple;
//     Cube *cube = __cube;

//     MddMemberRole *member_role = NULL;

//     if (__prefix_option)
//     {
//         short _type;
//         enum_oms _strat;
//         MemAllocMng *_mam;
//         obj_info(__prefix_option, &_type, &_strat, &_mam);

//         if (_type != OBJ_TYPE__MddMemberRole)
//         {
//             MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
//             thrd_mam->exception_desc = "exception: interpret_ast_mrf_parent :: __prefix_option is not a MddMemberRole.";
//             longjmp(thrd_mam->excep_ctx_env, -1);
//         }

//         member_role = __prefix_option;
//     }

//     if (member_role == NULL)
//         member_role = ids_mbrsdef__build(md_ctx, func_parent->ast_member, ctx_tuple, cube);

//     // when current member has no parent, return itself.
//     if (member_role->member->p_gid)
//         return mdd_mr__create(find_member_by_gid(member_role->member->p_gid), member_role->dim_role);
//     else
//         return member_role;
// }

// for ASTMemberFn_Parent
void *interpret_parent(void *md_ctx, void *mrole, void *parent, void *ctx_tuple, void *cube) {
    ASTMemberFn_Parent *func = parent;
    MddMemberRole *mr = mrole;

    if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
        if (!func->mr_up) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_parent.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }

        mr = up_evolving(md_ctx, func->mr_up, cube, ctx_tuple);
        if (!mr || obj_type_of(mr) != OBJ_TYPE__MddMemberRole) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: function: interpret_parent.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }
    }

    // when current member has no parent, return itself.
    if (mr->member->p_gid)
        return mdd_mr__create(find_member_by_gid(mr->member->p_gid), mr->dim_role);
    else
        return mr;
}