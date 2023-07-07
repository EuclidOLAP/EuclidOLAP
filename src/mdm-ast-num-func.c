#include <string.h>

#include "mdm-ast-num-func.h"
#include "md-model.h"
#include "mdd.h"
#include "math.h"
#include "vce.h"

// for ASTNumFunc_Avg
void *interpret_avg(void *md_ctx_, void *nil, void *avg_, void *ctx_tuple_, void *cube_)
{

    ASTNumFunc_Avg *avg = avg_;
    MddSet *set = ids_setdef__build(md_ctx_, avg->setdef, ctx_tuple_, cube_);
    unsigned int len = als_size(set->tuples);
    double sumval = 0;
    unsigned int valcount = 0;

    GridData *cell = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);

    for (int i = 0; i < len; i++)
    {
        MddTuple *tuple = tuple__merge(ctx_tuple_, als_get(set->tuples, i));
        if (avg->expdef)
        {
            Expression_evaluate(md_ctx_, avg->expdef, cube_, tuple, cell);
        }
        else
        {
            do_calculate_measure_value(md_ctx_, cube_, tuple, cell);
        }

        if (cell->type == GRIDDATA_TYPE_NUM && !cell->null_flag)
        {
            sumval += cell->val;
            valcount++;
        }
    }

    if (valcount == 0)
    {
        cell->type = GRIDDATA_TYPE_STR;
        cell->str = "Nan";
        return cell;
    }

    cell->type = GRIDDATA_TYPE_NUM;
    cell->null_flag = 0;

    if (avg->include_empty)
    {
        cell->val = sumval / als_size(set->tuples);
    }
    else
    {
        cell->val = sumval / valcount;
    }

    return cell;
}

// for ASTNumFunc_MaxMin
void *interpret_maxmin(void *md_ctx_, void *nil, void *mm, void *ctx_tuple_, void *cube_)
{

    ASTNumFunc_MaxMin *maxmin = mm;
    MddSet *set = ids_setdef__build(md_ctx_, maxmin->setdef, ctx_tuple_, cube_);
    unsigned int len = als_size(set->tuples);

    GridData cell;
    GridData *result = NULL;

    for (int i = 0; i < len; i++)
    {
        MddTuple *tuple = tuple__merge(ctx_tuple_, als_get(set->tuples, i));
        if (maxmin->expdef)
        {
            Expression_evaluate(md_ctx_, maxmin->expdef, cube_, tuple, &cell);
        }
        else
        {
            do_calculate_measure_value(md_ctx_, cube_, tuple, &cell);
        }

        if (cell.type != GRIDDATA_TYPE_NUM || cell.null_flag)
            continue;

        if (!result)
        {
            result = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);
            result->val = cell.val;
        }
        else
        {
            if (maxmin->opt == 'x')
            { // Max
                if (cell.val > result->val)
                    result->val = cell.val;
            }
            else
            { // Min
                if (cell.val < result->val)
                    result->val = cell.val;
            }
        }
    }

    if (!result)
    {
        result = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);
        result->null_flag = 1;
    }
    result->type = GRIDDATA_TYPE_NUM;
    return result;
}

// for ASTNumFunc_Aggregate
void *interpret_aggregate(void *md_ctx_, void *nil, void *agg, void *ctx_tuple_, void *cube_)
{

    ASTNumFunc_Aggregate *aggregate = agg;
    MddSet *set = ids_setdef__build(md_ctx_, aggregate->setdef, ctx_tuple_, cube_);
    int i, sz = als_size(set->tuples);
    GridData *cells = mam_alloc(sizeof(GridData) * sz, OBJ_TYPE__RAW_BYTES, NULL, 0);

    for (i = 0; i < sz; i++)
    {
        MddTuple *tuple = tuple__merge(ctx_tuple_, als_get(set->tuples, i));
        if (aggregate->expdef)
            Expression_evaluate(md_ctx_, aggregate->expdef, cube_, tuple, cells + i);
        else
            do_calculate_measure_value(md_ctx_, cube_, tuple, cells + i);
    }

    GridData *result = mam_alloc(sizeof(GridData), OBJ_TYPE__RAW_BYTES, NULL, 0);

    switch (aggregate->opt)
    {
    case FAO_COUNT:
        result->type = GRIDDATA_TYPE_NUM;
        result->val = sz;
        break;
    case FAO_MAX:
        cells_max(cells, sz, &result);
        result->type = GRIDDATA_TYPE_NUM;
        break;
    case FAO_MIN:
        cells_min(cells, sz, &result);
        result->type = GRIDDATA_TYPE_NUM;
        break;
    case FAO_DISTINCT_COUNT:
        result->type = GRIDDATA_TYPE_NUM;
        result->val = 1;
        for (int i = 1; i < sz; i++)
        {
            for (int j = 0; j < i; j++)
            {
                if (cells[i].type == GRIDDATA_TYPE_STR && cells[j].type == GRIDDATA_TYPE_STR && !strcmp(cells[i].str, cells[j].str))
                    goto bk_a;
            }
            result->val += 1;
        bk_a:
        }
        break;
    case FAO_DEFAULT:
    case FAO_SUM:
    default:
        cells_sum(cells, sz, &result);
        result->type = GRIDDATA_TYPE_NUM;
        break;
    }

    return result;
}

// for ASTNumFunc_Sum
void *interpret_sum(void *md_ctx_, void *nil, void *sum_, void *ctx_tuple_, void *cube_)
{

    ASTNumFunc_Sum *sum = sum_;

    GridData *grid_data = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);
    grid_data->null_flag = 1;
    grid_data->val = 0;
    grid_data->type = GRIDDATA_TYPE_NUM;

    MddSet *set = ids_setdef__build(md_ctx_, sum->setdef, ctx_tuple_, cube_);
    int i, sz = als_size(set->tuples);
    for (i = 0; i < sz; i++)
    {
        MddTuple *tuple = als_get(set->tuples, i);
        tuple = tuple__merge(ctx_tuple_, tuple);
        GridData tmp;
        if (sum->expdef)
            Expression_evaluate(md_ctx_, sum->expdef, cube_, tuple, &tmp);
        else
            do_calculate_measure_value(md_ctx_, cube_, tuple, &tmp);

        if (tmp.null_flag == 0)
        {
            grid_data->val += tmp.val;
            grid_data->null_flag = 0;
        }
        // else
        // {
        // 	grid_data->null_flag = 1;
        // 	return;
        // }
    }

    return grid_data;
}

// for ASTNumFunc_Count
void *interpret_count(void *md_ctx_, void *nil, void *count_, void *ctx_tuple_, void *cube_)
{

    ASTNumFunc_Count *count = count_;

    GridData *grid_data = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);
    grid_data->null_flag = 0;
    grid_data->val = 0;

    MddSet *set = ids_setdef__build(md_ctx_, count->setdef, ctx_tuple_, cube_);

    int i, tuples_size = als_size(set->tuples);

    if (count->include_empty)
    {
        grid_data->val = tuples_size;
        return grid_data;
    }

    MddTuple **tuples_matrix_h = mam_alloc(sizeof(MddTuple *) * tuples_size, OBJ_TYPE__RAW_BYTES, NULL, 0);

    for (i = 0; i < tuples_size; i++)
    {
        tuples_matrix_h[i] = tuple__merge(ctx_tuple_, (MddTuple *)(als_get(set->tuples, i)));
    }

    // char *null_flags;
    ArrayList *grids = vce_vactors_values(md_ctx_, tuples_matrix_h, tuples_size);

    for (i = 0; i < tuples_size; i++)
    {
        GridData *gd = als_get(grids, i);
        if (gd->null_flag == 0)
            grid_data->val += 1;
    }

    return grid_data;
}

// for ASTNumFunc_Median
void *interpret_median(void *md_ctx_, void *nil, void *median_, void *ctx_tuple_, void *cube_)
{

    ASTNumFunc_Median *median = median_;

    MddSet *set = ids_setdef__build(md_ctx_, median->setdef, ctx_tuple_, cube_);
    int i, sz = als_size(set->tuples);

    GridData *cells = mam_alloc(sizeof(GridData) * sz, OBJ_TYPE__GridData, NULL, 0);

    for (i = 0; i < sz; i++)
    {
        MddTuple *tuple = tuple__merge(ctx_tuple_, als_get(set->tuples, i));

        if (median->expdef)
            Expression_evaluate(md_ctx_, median->expdef, cube_, tuple, cells + i);
        else
            do_calculate_measure_value(md_ctx_, cube_, tuple, cells + i);
    }

    double tmpv;
    for (i = 0; i < sz - 1; i++)
    {
        for (int j = i + 1; j < sz; j++)
        {
            if (cells[j].val < cells[i].val)
            {
                tmpv = cells[j].val;
                cells[j].val = cells[i].val;
                cells[i].val = tmpv;
            }
        }
    }

    if (sz % 2 == 0)
    {
        cells[sz / 2].val = (cells[sz / 2 - 1].val + cells[sz / 2].val) / 2;
    }

    return cells + (sz / 2);
}

// for ASTNumFunc_Rank
void *interpret_rank(void *md_ctx_, void *nil, void *rank_, void *ctx_tuple_, void *cube_)
{
    ASTNumFunc_Rank *rank = rank_;
    MddTuple *tuple = NULL;
    if (obj_type_of(rank->param1) == OBJ_TYPE__TupleDef)
    {
        tuple = ids_tupledef__build(md_ctx_, rank->param1, ctx_tuple_, cube_);
    }
    else
    {
        MddMemberRole *mr = up_evolving(md_ctx_, rank->param1, cube_, ctx_tuple_);
        tuple = mdd_tp__create();
        mdd_tp__add_mbrole(tuple, mr);
    }

    MddSet *set = ids_setdef__build(md_ctx_, rank->setdef, ctx_tuple_, cube_);
    unsigned sz = als_size(set->tuples);

    if (rank->expdef)
    {
        GridData *cells = mam_alloc(sizeof(GridData) * sz, OBJ_TYPE__GridData, NULL, 0);
        for (int i = 0; i < sz; i++)
        {
            MddTuple *tup = tuple__merge(ctx_tuple_, als_get(set->tuples, i));
            Expression_evaluate(md_ctx_, rank->expdef, cube_, tup, cells + i);
        }

        double cval;
        MddTuple *ctup = NULL;

        for (int i = 0; i < sz - 1; i++)
        {
            for (int j = i + 1; j < sz; j++)
            {
                if (cells[j].val < cells[i].val)
                {
                    cval = cells[j].val;
                    cells[j].val = cells[i].val;
                    cells[i].val = cval;

                    ctup = als_get(set->tuples, i);
                    ArrayList_set(set->tuples, i, als_get(set->tuples, j));
                    ArrayList_set(set->tuples, j, ctup);
                }
            }
        }
    }

    GridData *res = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);
    res->type = GRIDDATA_TYPE_NUM;
    for (int i = 0; i < sz; i++)
    {
        if (Tuple__cmp(tuple, als_get(set->tuples, i)) == 0)
        {
            res->val = i + 1;
            return res;
        }
    }
    res->val = 0;
    return res;
}