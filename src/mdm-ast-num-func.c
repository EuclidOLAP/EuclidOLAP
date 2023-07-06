#include "mdm-ast-num-func.h"
#include "md-model.h"
#include "mdd.h"
#include "vce.h"

// for ASTNumFunc_Avg
void *interpret_avg(void *md_ctx_, void *nil, void *avg_, void *ctx_tuple_, void *cube_) {

    ASTNumFunc_Avg *avg = avg_;
    MddSet *set = ids_setdef__build(md_ctx_, avg->setdef, ctx_tuple_, cube_);
    unsigned int len = als_size(set->tuples);
    double sumval = 0;
    unsigned int valcount = 0;

    GridData *cell = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);

    for (int i = 0; i < len; i++) {
        MddTuple *tuple = tuple__merge(ctx_tuple_, als_get(set->tuples, i));
        if (avg->expdef) {
			Expression_evaluate(md_ctx_, avg->expdef, cube_, tuple, cell);
        } else {
			do_calculate_measure_value(md_ctx_, cube_, tuple, cell);
        }

        if (cell->type == GRIDDATA_TYPE_NUM && !cell->null_flag) {
            sumval += cell->val;
            valcount++;
        }
    }

    if (valcount == 0) {
        cell->type = GRIDDATA_TYPE_STR;
        cell->str = "Nan";
        return cell;
    }

    cell->type = GRIDDATA_TYPE_NUM;
    cell->null_flag = 0;

    if (avg->include_empty) {
        cell->val = sumval / als_size(set->tuples);
    } else {
        cell->val = sumval / valcount;
    }

    return cell;
}