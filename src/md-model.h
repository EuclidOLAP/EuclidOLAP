#ifndef MULTIDIMENSIONAL_MODEL__H
#define MULTIDIMENSIONAL_MODEL__H 1

#include "utils.h"
// #include "command.h"
#include "mdx.h"
// #include "tools/elastic-byte-buffer.h"

typedef long md_gid;

#define MD_ENTITY_NAME_BYTSZ 128

typedef struct _stct_dim_
{
	md_gid gid;
	md_gid def_hierarchy_gid;
	char name[MD_ENTITY_NAME_BYTSZ];
} Dimension;


#define MDD_MEMBER__BIN_ATTR_FLAG__NON_LEAF 1

typedef struct mdm_member
{
	char name[MD_ENTITY_NAME_BYTSZ];
	md_gid gid;
	md_gid p_gid;
	md_gid dim_gid;

	// If the current member is not in the default hierarchy of the dimension, 
	// the link attribute points to the equivalent member under the default hierarchy.
	md_gid link;

	// The ID of the hierarchy in which this dimension member was created.
	md_gid hierarchy_gid;

	unsigned short lv;

	// Each binary bit represents an attribute switch.
	// lowest bit, 0 - leaf member, 1 - non-leaf member.
	int bin_attr;

	// abs_path is a data block of length 'lv * sizeof(md_gid)' bytes.
	md_gid *abs_path;
} Member;


typedef struct level_
{
	md_gid gid;
	md_gid dim_gid;
	md_gid hierarchy_gid;
	char name[MD_ENTITY_NAME_BYTSZ];
	unsigned int level;
} Level;


typedef struct _euclid_cube_stct_
{
	md_gid gid;
	char name[MD_ENTITY_NAME_BYTSZ];
	ArrayList *dim_role_ls;
	Dimension *measure_dim;
	ArrayList *measure_mbrs;
} Cube;

#define DR_MEASURE_MASK 0x01

typedef struct _dim_role_stct_
{
	md_gid gid;
	char name[MD_ENTITY_NAME_BYTSZ];
	md_gid cube_gid;
	md_gid dim_gid;
	int sn; // sequence number

	/*
	 * lowest bit: 0 - non measure dimension role, 1 - measure
	 */
	int bin_attr;
} DimensionRole;



typedef struct multidimensional_result
{
	ArrayList *axes;
	ArrayList *grids;
	// double *vals;
	// char *null_flags;
	// unsigned long rs_len;
} MultiDimResult;
MultiDimResult *MultiDimResult_creat();


typedef struct mdd_tuple
{
	ArrayList *mr_ls;
	int attachment; // There is no fixed meaning, it is used according to the program context.
} MddTuple;


typedef struct mdd_set
{
	ArrayList *tuples;
} MddSet;


typedef struct mdd_axis
{
	MddSet *set;
	unsigned short posi;
} MddAxis;


typedef struct mdd_mbr_role
{
	Member *member;
	MemberFormula *member_formula;
	DimensionRole *dim_role;
} MddMemberRole;


typedef struct Level_Role_
{
	Level *lv;
	DimensionRole *dim_role;
} LevelRole;


typedef struct mdm_hierarchy
{
	md_gid gid;
	md_gid dimension_gid;
	char name[MD_ENTITY_NAME_BYTSZ];
} Hierarchy;

typedef struct mdm_hierarchy_role
{
	DimensionRole *dim_role;
	Hierarchy *hierarchy;
} HierarchyRole;

#endif