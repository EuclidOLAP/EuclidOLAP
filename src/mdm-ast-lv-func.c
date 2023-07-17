// #include <string.h>
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