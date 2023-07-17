#include <string.h>
// #include <math.h>

// #include "mdm-ast-num-func.h"
// #include "md-model.h"
#include "mdd.h"
// #include "mathematics.h"
#include "mdm-ast-lv-func.h"

extern ArrayList *dims_pool;
extern ArrayList *hierarchies_pool;
extern ArrayList *member_pool;
extern ArrayList *cubes_pool;
extern ArrayList *levels_pool;

// for ASTLvFunc_Level
void *interpret_Level(void *md_ctx_, void *mr, void *lv_fn, void *ctx_tuple_, void *cube_) {
    ASTLvFunc_Level *lv_func = lv_fn;

	MddMemberRole *mrole = mr;
	if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
	    mrole = up_evolving(md_ctx_, lv_func->mrdef, cube_, ctx_tuple_);
        if (!mrole || obj_type_of(mrole) != OBJ_TYPE__MddMemberRole) {
            MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
            thrd_mam->exception_desc = "exception: interpret_Level - The member cannot be determined.";
            longjmp(thrd_mam->excep_ctx_env, -1);
        }
	}

    unsigned int lvsz = als_size(levels_pool);
    for (int i=0;i<lvsz;i++) {
        Level *level = als_get(levels_pool, i);
        if (level->hierarchy_gid == mrole->member->hierarchy_gid && level->level == mrole->member->lv) {
            return LevelRole_creat(level, mrole->dim_role);
        }
    }

    return NULL;
}

// for ASTLvFunc_Levels
void *interpret_Levels(void *md_ctx_, void *dhr, void *lvs_fn, void *ctx_tuple_, void *cube_) {
    ASTLvFunc_Levels *lvs = lvs_fn;
    
	DimensionRole *drole = NULL;
	Hierarchy *hy = NULL;
    HierarchyRole *hyrole = NULL;

	if (dhr && obj_type_of(dhr) == OBJ_TYPE__HierarchyRole) {
		hyrole = dhr;
		hy = hyrole->hierarchy;
		drole = hyrole->dim_role;
		goto p0;
	}

	if (dhr && obj_type_of(dhr) == OBJ_TYPE__DimensionRole) {
		drole = dhr;
		Dimension *dim = find_dim_by_gid(drole->dim_gid);
		hy = find_hierarchy(dim->def_hierarchy_gid);
		goto p0;
	}

	void *unrole = up_evolving(md_ctx_, lvs->dhdef, cube_, ctx_tuple_);

	if (unrole && obj_type_of(unrole) == OBJ_TYPE__HierarchyRole) {
		hyrole = unrole;
		hy = hyrole->hierarchy;
		drole = hyrole->dim_role;
		goto p0;
	}

	if (unrole && obj_type_of(unrole) == OBJ_TYPE__DimensionRole) {
		drole = unrole;
		Dimension *dim = find_dim_by_gid(drole->dim_gid);
		hy = find_hierarchy(dim->def_hierarchy_gid);
		goto p0;
	}

p0:

    GridData cell;
    Expression_evaluate(md_ctx_, lvs->lvexp, cube_, ctx_tuple_, &cell);


	for (int i=0; i<als_size(levels_pool) ;i++) {
		Level *lv = als_get(levels_pool, i);
        if (lv->hierarchy_gid != hy->gid)
            continue;
        if (cell.type == GRIDDATA_TYPE_NUM && lv->level == (unsigned int)cell.val) {
            return LevelRole_creat(lv, drole); 
        }
        if (cell.type == GRIDDATA_TYPE_STR && !strcmp(cell.str, lv->name)) {
            return LevelRole_creat(lv, drole); 
        }
	}

    MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
    thrd_mam->exception_desc = "exception: interpret_Levels - Level obj not found.";
    longjmp(thrd_mam->excep_ctx_env, -1);

}