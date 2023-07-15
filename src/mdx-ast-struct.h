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
#define MEMBER_DEF__UNIVERSALPATH 3

typedef struct member_definition
{
    ids_ct t_cons;
    ArrayList *mbr_abs_path;
    void *member_fn;
    MDMEntityUniversalPath *eup;
} MemberDef;


typedef struct term_expression
{
    ArrayList *plus_terms;
    ArrayList *minus_terms;
} Expression;


// The common head of AST function defines.
typedef struct _ast_function_common_head_
{
    /*
     * @param  md_context      - MDContext
     * @param  prefix_option   - Represents the default entity option when the function is a suffix,
     *                           the default option may be a multidimensional model entity
     *                           (dimension, hierarchy, level, member) or a set or tuple.
     *                           In the case of a logical function, this parameter represents a GridData pointer.
     * @param  ast_member_func - AST member function define
     * @param  context_tuple   - MddTuple
     * @param  cube            - Cube
     * @return
     */
    void *(* interpret)(void *md_context, void *prefix_option, void *ast_member_func, void *context_tuple, void *cube);
} ASTFunctionCommonHead;


typedef struct ast_str_func_name
{
    ASTFunctionCommonHead head;

    MDMEntityUniversalPath *up;
} ASTStrFunc_Name;

typedef enum ast_str_exp_type {
    STR_LITERAL,
    STR_FUNC
} strexptype;

typedef struct ast_str_exp
{
    ASTFunctionCommonHead head;
    strexptype type;
    union {
        char *str;
        void *str_func;
    } part;
} ASTStrExp;

#endif