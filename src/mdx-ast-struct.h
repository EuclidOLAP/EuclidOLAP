#ifndef MDX_AST_STRUCT__H
#define MDX_AST_STRUCT__H 1

#include "utils.h"

#define MEU_SEG_TYPE_TXT   't'
#define MEU_SEG_TYPE_ID    'i'
#define MEU_SEG_TYPE_STAMP 's'

// Represents a multidimensional model entity that displays a definition.
typedef struct _mdm_entity_universal_path_ {
    ArrayList *list;
} MDMEntityUniversalPath;

typedef struct _mdmentityupsegment_
{
    union {
        char *seg_str;
        unsigned long mde_global_id;
        unsigned long mde_timestamp;
    } info;
    char type;
} MdmEntityUpSegment;


typedef unsigned char ids_ct;


#define MEMBER_DEF__MBR_ABS_PATH 1
#define MEMBER_DEF__MBR_FUNCTION 2

typedef struct member_definition
{
    ids_ct t_cons;
    ArrayList *mbr_abs_path;
    void *member_fn;
} MemberDef;


// The common head of AST member function define.
typedef struct _ast_common_member_func_
{
    /*
     * @param  md_context      - MDContext
     * @param  prefix_option   - Represents default option when shortened form
     * @param  ast_member_func - AST member function define
     * @param  context_tuple   - MddTuple
     * @param  cube            - Cube
     * @return MddMemberRole
     */
    void *(* interpret)(void *md_context, void *prefix_option, void *ast_member_func, void *context_tuple, void *cube);
} ASTCommonMemberFunc;

typedef struct _ast_member_func_parent_
{
    ASTCommonMemberFunc head;
    MemberDef *ast_member;
} ASTMemberFunc_Parent;

// TODO Schedule removal
typedef struct _member_role_func_parent_ {
    char suf_flag;
    MDMEntityUniversalPath *hierarchy;
} MemberRoleFuncParent;

#endif