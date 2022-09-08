#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mdd.h"
#include "vce.h"
#include "utils.h"

static ArrayList *coor_sys_ls;
static ArrayList *space_ls;

static void _do_calculate_measure_value(MDContext *md_ctx, MeasureSpace *space, ArrayList *sor_ls, int deep, unsigned long offset, GridData *grid_data, int mea_val_idx);

static void MeasureSpace_coordinate_intersection_value(MeasureSpace *space, unsigned long index, int mea_val_idx, GridData *gridData);

void vce_init()
{
    coor_sys_ls = als_create(16, "CoordinateSystem *");
    space_ls = als_create(16, "MeasureSpace *");
}

void vce_load() {
    FILE *cubes_fd = open_file(META_DEF_CUBES_FILE_PATH, "r");

	md_gid cube_id;
	while (fread((void *)&cube_id, sizeof(md_gid), 1, cubes_fd) > 0) {

        char src_dir[128];
        memset(src_dir, 0, 128);
        getcwd(src_dir, 80);

        char profile_path[128];
        memset(profile_path, 0, 128);
        sprintf(profile_path, "%s/data/profile-%lu", src_dir, cube_id);

        if (access(profile_path, F_OK) == 0)
            reload_space(cube_id);
    }

    fclose(cubes_fd);
}

/* TODO
 * The bug are not urgent, but need to be fixed.
 * When inserting new detailed measure data, the order in which it is defined must be the same as the
 * order of dimension roles when building the cube, otherwise the data will not be able to be found.
 */
int vce_append(EuclidCommand *ec)
{
    char *bytes = ec->bytes;
    unsigned int pkg_capacity = *((unsigned int *)bytes);
    size_t i = sizeof(int) + sizeof(short);

    unsigned long cs_id = *((unsigned long *)slide_over_mem(bytes, sizeof(long), &i));
    unsigned int axes_count = *((unsigned int *)slide_over_mem(bytes, sizeof(int), &i));
    unsigned int vals_count = *((unsigned int *)slide_over_mem(bytes, sizeof(int), &i));

    char src_dir[128];
	memset(src_dir, 0, 128);
	getcwd(src_dir, 80);

    char profile[128];
	memset(profile, 0, 128);
    sprintf(profile, "data/profile-%lu", cs_id);

    char profile_path[128];
	memset(profile_path, 0, 128);
    sprintf(profile_path, "%s/%s", src_dir, profile);

    if (access(profile_path, F_OK) != 0) {
        // create persistent file about the CoordinateSystem
        append_file_data(profile, (void *)&cs_id, sizeof(cs_id));
        append_file_data(profile, (void *)&axes_count, sizeof(axes_count));
        append_file_data(profile, (void *)&vals_count, sizeof(vals_count));
    }

    // Store the newly added measure data and its positioning information in the data file.
    char data_file[128];
    sprintf(data_file, "data/data-%lu", cs_id);
    int _offset = sizeof(pkg_capacity) + sizeof(intent) + sizeof(cs_id) + sizeof(axes_count) + sizeof(vals_count);
    append_file_data(data_file, (void *)(bytes + _offset), pkg_capacity - _offset);

    reload_space(cs_id);
    return 0;
}

static void *Scale_print__rbt(RBNode *node, void *param) {
    Scale_print(node->obj);
    return NULL;
}

static void *ScaleOffsetRange_print__rbt(RBNode *node, void *param) {
    ScaleOffsetRange_print(node->obj);
    return NULL;
}

void reload_space(unsigned long cs_id) {
    space_unload(cs_id);

    unsigned char tmpbuf[0x01 << 10]; // 1024

    char profile[128];
    sprintf(profile, "data/profile-%lu", cs_id);

    unsigned int axes_count;
    unsigned int vals_count;

    FILE *pfd = open_file(profile, "r");
    fread(tmpbuf, sizeof(cs_id), 1, pfd);
    fread(&axes_count, sizeof(axes_count), 1, pfd);
    fread(&vals_count, sizeof(vals_count), 1, pfd);
    fclose(pfd);

    CoordinateSystem *cs;
    int i, csz = als_size(coor_sys_ls);
    for (i = 0; i < csz; i++)
    {
        cs = als_get(coor_sys_ls, i);
        if (cs->id == cs_id)
            goto cs_completed;
    }

    // Create a coordinate system object in memory
    cs = cs_create(&cs_id);
    for (i = 0; i < axes_count; i++)
    {
        Axis *axis = ax_create();
        cs_add_axis(cs, axis);
    }
    als_add(coor_sys_ls, cs);

cs_completed:

    i = i;

    char data_file[128];
    sprintf(data_file, "data/data-%lu", cs_id);
    FILE *data_fd = open_file(data_file, "r");
    int coor_pointer_len;

    while (fread(&coor_pointer_len, sizeof(int), 1, data_fd) > 0) {
        fread(tmpbuf, coor_pointer_len * sizeof(__uint64_t), 1, data_fd);
        // At the same time, hang the scale objects on the corresponding axis in the coordinate system object.
        Scale *scale = scal_create();
        scal_put_fragments(scale, coor_pointer_len, tmpbuf);
        Axis *axis = cs_get_axis(cs, 0);
        ax_set_scale(axis, scale);

        for (i=1;i<axes_count;i++) {
            fread(&coor_pointer_len, sizeof(int), 1, data_fd);
            fread(tmpbuf, coor_pointer_len * sizeof(__uint64_t), 1, data_fd);
            Scale *scale = scal_create();
            scal_put_fragments(scale, coor_pointer_len, tmpbuf);
            Axis *axis = cs_get_axis(cs, i);
            ax_set_scale(axis, scale);
        }

        // skip some bytes
        fread(tmpbuf, (sizeof(double) + sizeof(char)) * vals_count, 1, data_fd);
    }
    fclose(data_fd);

    // Calculate the actual size of the multidimensional array corresponding to the coordinate system.
    unsigned long space_capacity = 1;
    for (i = 0; i < axes_count; i++)
    {
        Axis *axis = cs_get_axis(cs, i);
        ax_reordering(axis);
        int sz = ax_size(axis);

        // TODO FIXME
        // FIXME May exceed the maximum value of the long data type, if so, you need to customize the data type.
        space_capacity *= sz;
    }

    size_t space_partition_count = space_capacity / SPACE_DEF_PARTITION_SPAN_MIN;
    if (space_partition_count == 0) {
        space_partition_count = 1;
    } else if (space_capacity % SPACE_DEF_PARTITION_SPAN_MIN) {
        ++space_partition_count;
    }

    printf("[debug] space_capacity < %lu >, SPACE_DEF_PARTITION_COUNT < %d >, space_partition_count < %lu >\n", space_capacity, SPACE_DEF_PARTITION_COUNT, space_partition_count);

    // Creates a new logical multidimensional array object in memory.
    MeasureSpace *space = space_create(space_partition_count, SPACE_DEF_PARTITION_SPAN_MIN, vals_count);
    space->id = cs_id;

    // Traverse the data file and insert all measure data into the logical multidimensional array.
    data_fd = open_file(data_file, "r");

    while (1)
    {
        __uint64_t measure_space_idx = 0;
        for (i = 0; i < axes_count; i++)
        {
            if (fread(tmpbuf, sizeof(int), 1, data_fd) < 1)
                goto finished;

            int scale_len = *((int *)tmpbuf);

            fread(tmpbuf, sizeof(md_gid), scale_len, data_fd);
            Axis *axis = cs_get_axis(cs, i);
            __uint64_t sc_posi = ax_scale_position(axis, scale_len, tmpbuf);
            __uint64_t ax_span = cs_axis_span(cs, i);
            measure_space_idx += sc_posi * ax_span;
        }
        size_t cell_mem_sz = vals_count * (sizeof(double) + sizeof(char));
        void *cell = __objAlloc__(sizeof(measure_space_idx) + cell_mem_sz, OBJ_TYPE__RAW_BYTES);
        *((unsigned long *)cell) = measure_space_idx;
        fread(cell + sizeof(measure_space_idx), cell_mem_sz, 1, data_fd);

        space_add_measure(space, measure_space_idx, cell);
    }

finished:
    fclose(data_fd);

    space_plan(space);

    CoordinateSystem__gen_auxiliary_index(cs);
    CoordinateSystem__calculate_offset(cs);

    als_add(space_ls, space);

}

CoordinateSystem *cs_create(__uint64_t *id_addr)
{
    CoordinateSystem *cs = __objAlloc__(sizeof(CoordinateSystem), OBJ_TYPE__CoordinateSystem);
    if (id_addr != NULL)
        cs->id = *id_addr;
    cs->axes = als_create(32, "Axis *");
    return cs;
}

Axis *ax_create()
{
    Axis *ax = __objAlloc__(sizeof(Axis), OBJ_TYPE__Axis);
    ax->rbtree = rbt_create("struct _axis_scale *", scal_cmp, scal__destory);
    ax->sor_idx_tree = rbt_create("ScaleOffsetRange *", ScaleOffsetRange_cmp, ScaleOffsetRange_destory);
    return ax;
}

Axis *cs_get_axis(CoordinateSystem *cs, int axis_position)
{
    return als_get(cs->axes, axis_position);
}

void *scal__destory(void *scale)
{
    Scale *s = (Scale *)scale;
    // _release_mem_(s->fragments);
    // _release_mem_(s);
}

Scale *scal_create()
{
    return __objAlloc__(sizeof(Scale), OBJ_TYPE__Scale);
}

void cs_add_axis(CoordinateSystem *cs, Axis *axis)
{
    als_add(cs->axes, axis);
}

void scal_put_fragments(Scale *scale, int fgs_len, void *fragments)
{
    scale->fragments_len = fgs_len;
    scale->fragments = __objAlloc__(fgs_len * sizeof(__uint64_t), OBJ_TYPE__RAW_BYTES);
    memcpy(scale->fragments, fragments, fgs_len * sizeof(__uint64_t));
}

Scale *scal__alloc(int fgs_len, void *fragments)
{
    Scale *s = scal_create();
    scal_put_fragments(s, fgs_len, fragments);
    return s;
}

void ax_set_scale(Axis *axis, Scale *scale)
{
    // Scale_print(scale);
    rbt_add(axis->rbtree, scale);
}

int scal_cmp(void *_one, void *_other)
{
    Scale *one = (Scale *)_one, *other = (Scale *)_other;

    int i, len_c = one->fragments_len < other->fragments_len ? one->fragments_len : other->fragments_len;
    __uint64_t *f = one->fragments;
    __uint64_t *o_f = other->fragments;
    for (i = 0; i < len_c; i++)
    {
        if (*(f + i) < *(o_f + i))
            return -1;
        if (*(f + i) > *(o_f + i))
            return 1;
    }
    return 0;
}

void space_unload(__uint64_t id)
{
    int i;
    for (i = 0; i < als_size(space_ls); i++)
    {
        MeasureSpace *space = (MeasureSpace *)als_get(space_ls, i);
        if (space->id == id)
        {
            als_remove(space_ls, space);
            space__destory(space);
            break;
        }
    }
}

void ax_reordering(Axis *axis)
{
    rbt__reordering(axis->rbtree);
}

int ax_size(Axis *axis)
{
    return rbt__size(axis->rbtree);
}

/**
 * @param cell is a block of memory
 *
 * cell structure:
 * 8 bytes - position
 * (
 *   8 bytes - measure value
 *   1 byte - null flag
 * )+
 *
 */
// TODO [2022-5-17 19:43:32] There seems to be a problem with this code
int cell_cmp(void *cell, void *other)
{
    unsigned long c_posi = *((unsigned long *)cell);
    unsigned long o_posi = *((unsigned long *)other);
    if (o_posi > c_posi)
        return -1;
    if (o_posi < c_posi)
        return 1;
    return 0;
}

static void *_cell__destory(void *cell)
{
    // _release_mem_(cell);
}

MeasureSpace *space_create(size_t segment_count, size_t segment_scope, int cell_vals_count)
{
    MeasureSpace *s = __objAlloc__(sizeof(MeasureSpace), OBJ_TYPE__MeasureSpace);
    s->cell_vals_count = cell_vals_count;
    s->segment_count = segment_count;
    s->segment_scope = segment_scope;
    s->tree_ls_h = __objAlloc__(sizeof(RedBlackTree *) * segment_count, OBJ_TYPE__RedBlackTree);
    s->data_ls_h = __objAlloc__(sizeof(void *) * segment_count, OBJ_TYPE__RAW_BYTES);
    s->data_lens = __objAlloc__(sizeof(unsigned long) * segment_count, OBJ_TYPE__RAW_BYTES);

    int i;
    for (i = 0; i < segment_count; i++)
        s->tree_ls_h[i] = rbt_create("*cell", cell_cmp, _cell__destory);

    return s;
}

void *build_space_measure(RBNode *node, void *callback_params)
{
    int cell_size = *((int *)callback_params) + sizeof(long); // bytes count
    void *block_addr = *((void **)(callback_params + sizeof(int)));
    memcpy(block_addr + cell_size * (node->index), node->obj, cell_size);
}

void space_plan(MeasureSpace *space)
{
    int i;
    for (i = 0; i < space->segment_count; i++)
    {
        RedBlackTree *tree = space->tree_ls_h[i];
        rbt__reordering(tree);

        int cell_size = space->cell_vals_count * (sizeof(double) + sizeof(char));
        unsigned int actual_cells_sz = rbt__size(tree);
        space->data_lens[i] = actual_cells_sz;
        int posi_cell_sz = (sizeof(unsigned long) + cell_size);
        space->data_ls_h[i] = __objAlloc__(actual_cells_sz * posi_cell_sz, OBJ_TYPE__RAW_BYTES);

        char callback_params[sizeof(int) + sizeof(void *)];
        *((int *)callback_params) = cell_size;
        memcpy(&(callback_params[sizeof(int)]), space->data_ls_h + i, sizeof(void *));
        rbt__scan_do(tree, callback_params, build_space_measure);
        rbt__clear(tree);
    }
}

__uint64_t ax_scale_position(Axis *axis, int fgs_len, void *fragments)
{
    Scale *s = scal__alloc(fgs_len, fragments);
    // Scale_print(s);
    RBNode *n = rbt__find(axis->rbtree, s);
    return n->index;
}

__uint64_t cs_axis_span(CoordinateSystem *cs, int axis_order)
{
    unsigned int axes_count = als_size(cs->axes);
    unsigned long span = 1;
    int i;
    for (i = axes_count - 1; i > axis_order; i--)
    {
        Axis *x = cs_get_axis(cs, i);
        span *= rbt__size(x->rbtree);
    }
    return span;
}

void space_add_measure(MeasureSpace *space, __uint64_t measure_position, void *cell)
{
    rbt_add(space->tree_ls_h[measure_position / space->segment_scope], cell);
}

void Scale_print(Scale *s)
{
    printf("Scale <%p> [ fragments_len = %d ] ", s, s->fragments_len);
    int i;
    for (i = 0; i < s->fragments_len; i++)
    {
        printf("  %lu", s->fragments[i]);
    }
    printf("\n");
}

void space__destory(MeasureSpace *s)
{
    size_t i;
    for (i = 0; i < s->segment_count; i++)
    {
        if (s->tree_ls_h[i])
            rbt__destory(s->tree_ls_h[i]);

        // if (s->data_ls_h[i])
        //     _release_mem_(s->data_ls_h[i]);
    }

    // _release_mem_(s->tree_ls_h);

    // _release_mem_(s->data_ls_h);

    // _release_mem_(s);
}

double *vce_vactors_values(MDContext *md_ctx, MddTuple **tuples_matrix_h, unsigned long v_len, char **null_flags)
{
    Cube *cube = Tuple_ctx_cube(tuples_matrix_h[0]);

    unsigned long i, j;

    CoordinateSystem *coor = NULL;
    for (i = 0; i < als_size(coor_sys_ls); i++)
    {
        if (((CoordinateSystem *)als_get(coor_sys_ls, i))->id == cube->gid)
        {
            coor = als_get(coor_sys_ls, i);
            break;
        }
    }

    for (i = 0; i < v_len; i++)
    {
        MddTuple *tuple = tuples_matrix_h[i];
        for (j = 0; j < als_size(tuple->mr_ls); j++)
        {
            MddMemberRole *mr = als_get(tuple->mr_ls, j);
            if (mr->member && (!mr->member->abs_path))
                mdd__gen_mbr_abs_path(mr->member);
            // else
            //     printf("[ info ] - This member role represents a calculation formula member.\n");
        }
    }

    double *result = __objAlloc__(v_len * sizeof(double), OBJ_TYPE__RAW_BYTES);
    *null_flags = __objAlloc__(v_len * sizeof(char), OBJ_TYPE__RAW_BYTES);

    GridData tmp;
    for (i = 0; i < v_len; i++)
    {
        do_calculate_measure_value(md_ctx, cube, tuples_matrix_h[i], &tmp);
        if (tmp.null_flag)
            (*null_flags)[i] = 1;
        else
            result[i] = tmp.val;
    }

    return result;
}

void *__set_ax_max_path_len(RBNode *node, void *param)
{
    Scale *scale = node->obj;
    unsigned int *ax_max_len = param;
    if (scale->fragments_len > *ax_max_len)
        *ax_max_len = scale->fragments_len;
    return NULL;
}

void *__Axis_build_index(RBNode *node, void *axis)
{
    Scale *scale = node->obj;
    Axis *ax = axis;
    memcpy(ax->index + node->index * ax->max_path_len * sizeof(md_gid), scale->fragments, scale->fragments_len * sizeof(md_gid));
    return NULL;
}

void CoordinateSystem__gen_auxiliary_index(CoordinateSystem *coor)
{
    unsigned int i, ax_sz = als_size(coor->axes);
    for (i = 0; i < ax_sz; i++)
    {
        Axis *ax = als_get(coor->axes, i);
        RedBlackTree *tree = ax->rbtree;
        rbt__scan_do(tree, &(ax->max_path_len), __set_ax_max_path_len);
        ax->index = __objAlloc__(rbt__size(tree) * ax->max_path_len * sizeof(md_gid), OBJ_TYPE__RAW_BYTES);
        rbt__scan_do(tree, ax, __Axis_build_index);
    }
}

void CoordinateSystem__calculate_offset(CoordinateSystem *coor)
{
    int i, ax_sz = als_size(coor->axes);
    Axis *ax_n, *ax = als_get(coor->axes, ax_sz - 1);
    ax->coor_offset = 1;

    for (i = ax_sz - 2; i >= 0; i--)
    {
        ax = als_get(coor->axes, i);
        ax_n = als_get(coor->axes, i + 1);
        ax->coor_offset = ax_size(ax_n) * ax_n->coor_offset;
    }

    int row, col;
    for (i = 0; i < ax_sz; i++)
    {
        Axis *axis = als_get(coor->axes, i);

        /* TODO
         * When the red-black tree adds elements, if there is another equal element, the red-black tree will not do anything,
         * which may cause some problems. Here, a new red-black tree is temporarily created, and this place needs to be
         * performed in the follow-up. Revise.
         */
        axis->sor_idx_tree = rbt_create("ScaleOffsetRange *", ScaleOffsetRange_cmp, ScaleOffsetRange_destory);
        int tree_sz = rbt__size(axis->rbtree);
        ScaleOffsetRange *sor = NULL;
        md_gid id, prev_id = 0;

        for (col = 0; col < axis->max_path_len; col++)
        {
            for (row = 0; row < tree_sz; row++)
            {
                id = ((md_gid *)axis->index)[row * axis->max_path_len + col];

                if (sor == NULL || id != prev_id)
                {
                    sor = ScaleOffsetRange_create();
                    sor->gid = id;
                    sor->offset = axis->coor_offset;
                    sor->end_position = sor->start_position = row;
                    sor->start_offset = sor->end_offset = row * axis->coor_offset;
                    rbt_add(axis->sor_idx_tree, sor);
                }
                else
                {
                    sor->end_position = row;
                    sor->end_offset = row * axis->coor_offset;
                }

                prev_id = id;
            }
        }
        rbt__reordering(axis->sor_idx_tree);
    }
}

void do_calculate_measure_value(MDContext *md_ctx, Cube *cube, MddTuple *tuple, GridData *grid_data)
{
    int i;
    int sz = als_size(tuple->mr_ls);

    for (i = sz - 1; i >= 0; i--)
    {
        MddMemberRole *mr = als_get(tuple->mr_ls, i);
        if (mr->member_formula)
        {
            Expression *exp = mr->member_formula->exp;
            Expression_evaluate(md_ctx, exp, cube, tuple, grid_data);
            return;
        }
    }

    ArrayList_sort(tuple->mr_ls, MddMemberRole_cmp);

    int coor_count = als_size(coor_sys_ls);

    CoordinateSystem *coor;

    for (i = 0; i < coor_count; i++)
    {
        coor = als_get(coor_sys_ls, i);
        if (coor->id == cube->gid)
        {
            break;
        }
        else
        {
            coor = NULL;
        }
    }

    ArrayList *sor_ls = als_create(64, "ScaleOffsetRange *");

    MddMemberRole *measure_mr;

    for (i = 0; i < sz; i++)
    {
        MddMemberRole *mr = als_get(tuple->mr_ls, i);
        DimensionRole *dr = mr->dim_role;

        // continue measure member role
        if (dr == NULL)
        {
            measure_mr = mr;
            continue;
        }

        Axis *ax = cs_get_axis(coor, i);

        ScaleOffsetRange *key = ScaleOffsetRange_create();
        key->gid = mr->member->gid;

        RBNode *node = rbt__find(ax->sor_idx_tree, key);

        if (node == NULL)
        {
            // If there is no corresponding coordinate, return a null value directly.
            grid_data->null_flag = 1;
            return;
        }

        ScaleOffsetRange *sor = (ScaleOffsetRange *)node->obj;

        als_add(sor_ls, sor);
    }

    int mea_val_idx = 0;
    Member *measure_m = measure_mr->member;
    for (i = 0; i < als_size(cube->measure_mbrs); i++)
    {
        Member *_m = (Member *)als_get(cube->measure_mbrs, i);
        if (measure_m->gid == _m->gid)
        {
            mea_val_idx = i;
            break;
        }
    }

    MeasureSpace *space;
    for (i = 0; i < als_size(space_ls); i++)
    {
        space = (MeasureSpace *)als_get(space_ls, i);
        if (space->id == cube->gid)
            break;
        else
            space = NULL;
    }

    memset(grid_data, 0, sizeof(GridData));
    grid_data->null_flag = 1; // measure value is default null
    _do_calculate_measure_value(md_ctx, space, sor_ls, 0, 0, grid_data, mea_val_idx);
}

static void _do_calculate_measure_value(MDContext *md_ctx, MeasureSpace *space, ArrayList *sor_ls, int deep, unsigned long offset, GridData *grid_data, int mea_val_idx)
{
    ScaleOffsetRange *sor = (ScaleOffsetRange *)als_get(sor_ls, deep);
    unsigned long _position;
    for (_position = sor->start_position; _position <= sor->end_position; _position++)
    {
        if (deep == (als_size(sor_ls) - 1))
        {
            GridData date;
            MeasureSpace_coordinate_intersection_value(space, offset + _position * sor->offset, mea_val_idx, &date);
            if (date.null_flag == 0)
            {
                grid_data->val += date.val;
                grid_data->null_flag = 0;
            }
        }
        else
        {
            _do_calculate_measure_value(md_ctx, space, sor_ls, deep + 1, offset + _position * sor->offset, grid_data, mea_val_idx);
        }
    }
}

ScaleOffsetRange *ScaleOffsetRange_create()
{
    return __objAlloc__(sizeof(ScaleOffsetRange), OBJ_TYPE__ScaleOffsetRange);
}

void ScaleOffsetRange_print(ScaleOffsetRange *sor)
{
    printf("[ ScaleOffsetRange %p ] gid = %lu, position [ %lu, %lu ], offset [ %lu, %lu ]\n", sor, sor->gid, sor->start_position, sor->end_position, sor->start_offset, sor->end_offset);
}

int ScaleOffsetRange_cmp(void *_obj, void *_other)
{
    ScaleOffsetRange *obj = (ScaleOffsetRange *)_obj;
    ScaleOffsetRange *other = (ScaleOffsetRange *)_other;
    return other->gid < obj->gid ? -1 : (other->gid > obj->gid ? 1 : 0);
}

void *ScaleOffsetRange_destory(void *sor)
{
    // TODO may 17 2022 - 20:00:05
    // printf("[ func - ScaleOffsetRange_destory ] This function has not been implemented yet.\n");
    return NULL;
}

static void MeasureSpace_coordinate_intersection_value(MeasureSpace *space, unsigned long index, int mea_val_idx, GridData *gridData)
{
    gridData->null_flag = 0; // default not null

    unsigned long segment_num = index / space->segment_scope;
    if (segment_num != 0 && !(index % space->segment_scope))
        --segment_num;

    unsigned long cells_count = space->data_lens[segment_num];

    unsigned long range_start = 0;
    unsigned long range_end = cells_count - 1;

    char *data = space->data_ls_h[segment_num];

    int a_cell_bytes_count = space->cell_vals_count * (sizeof(double) + sizeof(char)) + sizeof(unsigned long);

    unsigned long seg_cell_start_pos = *((unsigned long *)data);
    unsigned long seg_cell_end_pos = *((unsigned long *)(data + range_end * a_cell_bytes_count));

    if (index < seg_cell_start_pos || index > seg_cell_end_pos)
    {
        gridData->null_flag = 1;
        return;
    }

    while (1)
    {

        if (range_end - range_start > 1)
        {
            unsigned long range_mid = (range_start + range_end) / 2;
            unsigned long seg_cell_mid_pos = *((unsigned long *)(data + range_mid * a_cell_bytes_count));

            if (seg_cell_mid_pos == index)
            {
                void *grid_addr = data + range_mid * a_cell_bytes_count + sizeof(unsigned long) + (sizeof(double) + sizeof(char)) * mea_val_idx;
                if (*((char *)(grid_addr + sizeof(double))))
                {
                    gridData->null_flag = 1;
                }
                else
                {
                    gridData->val = *((double *)grid_addr);
                }
                return;
            }

            if (index < seg_cell_mid_pos)
                range_end = range_mid - 1;
            else
                range_start = range_mid + 1;
        }
        else
        {
            seg_cell_start_pos = *((unsigned long *)(data + range_start * a_cell_bytes_count));

            if (seg_cell_start_pos == index)
            {
                void *grid_addr = data + range_start * a_cell_bytes_count + sizeof(unsigned long) + (sizeof(double) + sizeof(char)) * mea_val_idx;
                if (*((char *)(grid_addr + sizeof(double))))
                {
                    gridData->null_flag = 1;
                }
                else
                {
                    gridData->val = *((double *)grid_addr);
                }
                return;
            }

            seg_cell_end_pos = *((unsigned long *)(data + range_end * a_cell_bytes_count));

            if (seg_cell_end_pos == index)
            {
                void *grid_addr = data + range_end * a_cell_bytes_count + sizeof(unsigned long) + (sizeof(double) + sizeof(char)) * mea_val_idx;
                if (*((char *)(grid_addr + sizeof(double))))
                {
                    gridData->null_flag = 1;
                }
                else
                {
                    gridData->val = *((double *)grid_addr);
                }
                return;
            }

            break;
        }
    }

    gridData->null_flag = 1;
}

void MeasureSpace_print(MeasureSpace *space) {
    printf(">>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    printf("id              - %lu\n", space->id);
    printf("segment_count   - %lu\n", space->segment_count);
    printf("segment_scope   - %lu\n", space->segment_scope);
    printf("cell_vals_count - %d\n", space->cell_vals_count);

    int cell_len = sizeof(long) + space->cell_vals_count * (sizeof(double) + sizeof(char));

    int i;
    for (i=0;i<space->segment_count;i++) {
        printf("\n");

        printf("%lu\n", space->data_lens[i]);
        int j;
        for (j=0;j<space->data_lens[i];j++) {
            void *cell = space->data_ls_h[i] + j * cell_len;
            printf("% 12lu  >  ", *((unsigned long *)cell));
            cell += sizeof(long);
            int k;
            for (k=0;k<space->cell_vals_count;k++) {
                printf("% 20lf - ", *((double *)(cell + k * (sizeof(double) + sizeof(char)))));
                printf("%d    ", *((char *)(cell + k * (sizeof(double) + sizeof(char)) + sizeof(double))));
            }
            printf("\n");
        }

        printf("\n");
    }

    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>>>>>>>>>\n");
    printf(">>>>>>>>\n");
}