#ifndef MDX_AST_STRUCT__H
#define MDX_AST_STRUCT__H 1

#include "utils.h"

#define MEU_SEG_TYPE_TXT   't'
#define MEU_SEG_TYPE_ID    'i'
#define MEU_SEG_TYPE_STAMP 's'

/*************************************************************************************
 *                                                                                   *
 *************************************************************************************/
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

#endif