#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <dirent.h>

#include "env.h"
#include "log.h"
#include "cfg.h"
#include "mdd.h"
#include "vce.h"
#include "utils.h"
#include "net.h"

#include "tools/elastic-byte-buffer.h"

extern OLAPEnv olap_env;

extern ArrayList *cubes_pool;

static MemAllocMng *vce_mam;
static ArrayList *space_mam_ls;
static ArrayList *coor_sys_ls;
static ArrayList *space_ls;

static void _do_calculate_measure_value(
    // MDContext *md_ctx,
    MeasureSpace *space, ArrayList *sor_ls, int deep, unsigned long offset, GridData *grid_data, int mea_val_idx);

/*
 * Aggregations based on dynamic sparse indexes.
 * The third parameter, 'dyc_uidx_len', represents the dynamic union index length,
 * which can be understood as the number of dimensions involved in the dynamic sparse dimension.
 */
static void agg_based_on_dynamic_sparse_idx
    (MeasureSpace *space, ArrayList *sor_ls, int dyc_uidx_len, int deep, unsigned long offset, GridData *grid_data, int mea_val_idx);

static void MeasureSpace_coordinate_intersection_value(MeasureSpace *space, unsigned long index, int mea_val_idx, GridData *gridData);

static void summarize_space_scope(MeasureSpace *space, unsigned long start_offset, unsigned long end_offset, int mea_val_idx, GridData *gridData);

static void calculates_the_full_summary_value(MeasureSpace *space, GridData *gridData, int mea_val_idx);

static void *__set_ax_max_path_len(RBNode *node, void *param);

static void *__Axis_build_index(RBNode *node, void *axis);

static void load_space(char *profile, char *data_file, unsigned long cs_id, CoordinateSystem **_cs, MeasureSpace **ms_);

static void load_mirror(char *coorsys_mirror, char *data_mirror, unsigned long cs_id, CoordinateSystem **_cs, MeasureSpace **ms_);

void vce_init()
{
    vce_mam = MemAllocMng_new();
    coor_sys_ls = als_new(16, "CoordinateSystem *", SPEC_MAM, vce_mam);
    space_ls = als_new(16, "MeasureSpace *", SPEC_MAM, vce_mam);
    space_mam_ls = als_new(16, "MemAllocMng *", SPEC_MAM, vce_mam);
}

void vce_load()
{

    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char *prefix = "data-";
    int prefix_len = strlen(prefix);
    char *suffix = NULL, *endptr = NULL;
    long cube_id = 0;

    assert(strlen(olap_env.OLAP_HOME) + 32 < 128);
    char f[128];
    memset(f, 0, 128);
    // getcwd(f, 80);
    // strcat(f, "/data");
    sprintf(f, "%s%s", olap_env.OLAP_HOME, "/data");

    dir = opendir(f);

    assert(dir != NULL);

    MemAllocMng *temp_mam = MemAllocMng_new();
    ArrayList *loaded_id_ls = als_new(20, "unsigned long *", SPEC_MAM, temp_mam);  

    while ((entry = readdir(dir)) != NULL)
    {
        if (strncmp(entry->d_name, "data-", strlen("data-")) == 0) {
            suffix = entry->d_name + strlen("data-");
        } else if (strncmp(entry->d_name, "mirror-data-", strlen("mirror-data-")) == 0) {
            suffix = entry->d_name + strlen("mirror-data-");
        } else {
            continue;
        }

        cube_id = strtol(suffix, &endptr, 10);
        for (int i=0; i<als_size(loaded_id_ls); i++) {
            if (cube_id == *(unsigned long *)als_get(loaded_id_ls, i)) {
                goto space_had_loaded;
            }
        }

        reload_space(cube_id);
        unsigned long *loaded_id = mam_alloc(sizeof(cube_id), OBJ_TYPE__RAW_BYTES, temp_mam, 0);
        *loaded_id = cube_id;
        als_add(loaded_id_ls, loaded_id);

space_had_loaded:
    }

    closedir(dir);

    mam_reset(temp_mam);
    obj_release(temp_mam->current_block);
    obj_release(temp_mam);
}

/* TODO
 * The bug are not urgent, but need to be fixed.
 * When inserting new detailed measure data, the order in which it is defined must be the same as the
 * order of dimension roles when building the cube, otherwise the data will not be able to be found.
 */
int vce_append(EuclidCommand *action)
{
    char *bytes = action->bytes;
    unsigned int pkg_capacity = *((unsigned int *)bytes);
    size_t i = sizeof(int) + sizeof(short);

    InsertingMeasuresOptions *options = (InsertingMeasuresOptions *)slide_over_mem(bytes, sizeof(InsertingMeasuresOptions), &i);

    unsigned long cs_id = *((unsigned long *)slide_over_mem(bytes, sizeof(long), &i));
    unsigned int axes_count = *((unsigned int *)slide_over_mem(bytes, sizeof(int), &i));
    unsigned int vals_count = *((unsigned int *)slide_over_mem(bytes, sizeof(int), &i));


    char cube_prof[256];
    assert(strlen(olap_env.OLAP_HOME) < 200);
    memset(cube_prof, 0, 256);
    sprintf(cube_prof, "%s/data/profile-%lu", olap_env.OLAP_HOME, cs_id);

    if (access(cube_prof, F_OK) != 0)
    {
        // create persistent file about the CoordinateSystem
        append_file_data(cube_prof, (void *)&cs_id, sizeof(cs_id));
        append_file_data(cube_prof, (void *)&axes_count, sizeof(axes_count));
        append_file_data(cube_prof, (void *)&vals_count, sizeof(vals_count));
    }


    // Store the newly added measure data and its positioning information in the data file.
    char data_file[256];
    assert(strlen(olap_env.OLAP_HOME) < 200);
    memset(data_file, 0, 256);
    sprintf(data_file, "%s/data/data-%lu", olap_env.OLAP_HOME, cs_id);
    // sprintf(data_file, "data/data-%lu", cs_id);
    int _offset = sizeof(pkg_capacity) + sizeof(intent) + sizeof(InsertingMeasuresOptions)
        + sizeof(cs_id) + sizeof(axes_count) + sizeof(vals_count);
    append_file_data(data_file, (void *)(bytes + _offset), pkg_capacity - _offset);

    if (options->re_mea_opt == ReloadMeasures_Enable)
        reload_space(cs_id);

    return 0;
}

static void *Scale_print__rbt(RBNode *node, void *param)
{
    Scale_print(node->obj);
    return NULL;
}

static void *ScaleOffsetRange_print__rbt(RBNode *node, void *param)
{
    ScaleOffsetRange_print(node->obj);
    return NULL;
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
int cell_cmp(void *cell, void *other)
{
    unsigned long c_posi = *((unsigned long *)cell);
    unsigned long o_posi = *((unsigned long *)other);
    return o_posi < c_posi ? -1 : (o_posi > c_posi ? 1 : 0);
}

static void *_cell__destory(void *cell)
{
    // _release_mem_(cell);
}

void *put_rbtelements_into_list(RBNode *node, void *param) {
    ArrayList *list = param;
    als_add(list, node->obj);
    return NULL;
}

void do_solidify_mirror(unsigned long cs_id) {

    assert(strlen(olap_env.OLAP_HOME) < 200);

    char coorsys_mirror[256], data_mirror[256];
    memset(coorsys_mirror, 0, 256);
    memset(data_mirror, 0, 256);
    sprintf(coorsys_mirror, "%s/data/mirror-coorsys-%lu", olap_env.OLAP_HOME, cs_id);
    sprintf(data_mirror, "%s/data/mirror-data-%lu", olap_env.OLAP_HOME, cs_id);

    if (access(coorsys_mirror, F_OK) == 0) {
        remove(coorsys_mirror);
    }

    if (access(data_mirror, F_OK) == 0) {
        remove(data_mirror);
    }

    CoordinateSystem *coorsys = NULL;
    MeasureSpace *measpace = NULL;

    for (int i = 0; i < als_size(coor_sys_ls); i++) {
        coorsys = coorsys ? coorsys : als_get(coor_sys_ls, i);
        measpace = measpace ? measpace : als_get(space_ls, i);
        if (coorsys->id != cs_id)
            coorsys = NULL;
        if (measpace->id != cs_id)
            measpace = NULL;
    }


    assert(coorsys && measpace);


    MemAllocMng *temp_mam = MemAllocMng_new();


    int cs_axes_count = als_size(coorsys->axes);

    for (int i=0; i<cs_axes_count; i++) {
        Axis *axis = als_get(coorsys->axes, i);
        ArrayList *tmpls = als_new(rbt__size(axis->sor_idx_tree), "ScaleOffsetRange *", SPEC_MAM, temp_mam);
        rbt__scan_do(axis->sor_idx_tree, tmpls, put_rbtelements_into_list);

        Axis *copyed_axis = mam_alloc(sizeof(Axis), OBJ_TYPE__Axis, temp_mam, 1);
        memcpy(copyed_axis, axis, sizeof(Axis));
        copyed_axis->sor_idx_tree = NULL;
        copyed_axis->assist = NULL;

        append_file_data(coorsys_mirror, (char *)copyed_axis, sizeof(Axis));
        for (int j=0; j<als_size(tmpls); j++) {
            append_file_data(coorsys_mirror, als_get(tmpls, j), sizeof(ScaleOffsetRange));
        }
    }


    MeasureSpace *copyed_space = mam_alloc(sizeof(MeasureSpace), OBJ_TYPE__MeasureSpace, temp_mam, 1);
    memcpy(copyed_space, measpace, sizeof(MeasureSpace));
    copyed_space->data_lens = NULL;
    copyed_space->data_ls_h = NULL;
    append_file_data(data_mirror, (char *)copyed_space, sizeof(MeasureSpace));

    append_file_data(data_mirror, (char *)(measpace->data_lens), measpace->segment_count * sizeof(unsigned long));

    for (size_t i=0; i<measpace->segment_count; i++) {
        append_file_data(data_mirror, measpace->data_ls_h[i], (sizeof(unsigned long) + measpace->cell_vals_count * (sizeof(double) + sizeof(char))) * measpace->data_lens[i]);
    }


    mam_reset(temp_mam);
    obj_release(temp_mam->current_block);
    obj_release(temp_mam);
}

static void load_mirror(char *coorsys_mirror, char *data_mirror, unsigned long cs_id, CoordinateSystem **_cs, MeasureSpace **ms_) {

    // Create a coordinate system object by MemAllocMng.
    MemAllocMng *space_mam = MemAllocMng_new();
    CoordinateSystem *coorsys = mam_alloc(sizeof(CoordinateSystem), OBJ_TYPE__CoordinateSystem, space_mam, 1);
    coorsys->id = cs_id;
    coorsys->axes = als_new(4, "Axis *", SPEC_MAM, space_mam);


    FILE *csm_fd = fopen(coorsys_mirror, "r");

    while (1) {
        Axis *ax = mam_alloc(sizeof(Axis), OBJ_TYPE__Axis, space_mam, 1);
        if (fread(ax, sizeof(Axis), 1, csm_fd) < 1)
            goto load_csm_done;

        ax->sor_idx_tree = rbt_create("ScaleOffsetRange *", ScaleOffsetRange_cmp, ScaleOffsetRange_destory, SPEC_MAM, space_mam);
        for (int i=0; i<ax->sor_idx_tree_size; i++) {
            ScaleOffsetRange *sor = mam_alloc(sizeof(ScaleOffsetRange), OBJ_TYPE__ScaleOffsetRange, space_mam, 0);
            fread(sor, sizeof(ScaleOffsetRange), 1, csm_fd);
            rbt_add(ax->sor_idx_tree, sor);
        }
        als_add(coorsys->axes, ax);
    }

load_csm_done:

    fclose(csm_fd);


    MeasureSpace *mea_space = mam_alloc(sizeof(MeasureSpace), OBJ_TYPE__MeasureSpace, space_mam, 1);

    FILE *sdm_fd = fopen(data_mirror, "r");
    fread(mea_space, sizeof(MeasureSpace), 1, sdm_fd);
    mea_space->data_ls_h = mam_alloc(sizeof(void *) * mea_space->segment_count, OBJ_TYPE__RAW_BYTES, space_mam, 0);
    mea_space->data_lens = mam_alloc(sizeof(unsigned long) * mea_space->segment_count, OBJ_TYPE__RAW_BYTES, space_mam, 0);


    fread(mea_space->data_lens, mea_space->segment_count * sizeof(unsigned long), 1, sdm_fd);

    for (size_t i=0; i<mea_space->segment_count; i++) {
        mea_space->data_ls_h[i] = mam_alloc(
            (sizeof(unsigned long) + mea_space->cell_vals_count * (sizeof(double) + sizeof(char))) * mea_space->data_lens[i],
            OBJ_TYPE__RAW_BYTES, space_mam, 0);

        fread(mea_space->data_ls_h[i],
            (sizeof(unsigned long) + mea_space->cell_vals_count * (sizeof(double) + sizeof(char))) * mea_space->data_lens[i],
            1, sdm_fd);
    }


    fclose(sdm_fd);


    *_cs = coorsys;
    *ms_ = mea_space;
}

static void load_space(char *profile, char *data_file, unsigned long cs_id, CoordinateSystem **_cs, MeasureSpace **ms_) {

    unsigned int axes_count, vals_count;


    ByteBuf *tmp_buf = buf__alloc(0x01UL << 10);


    FILE *profile_fd = fopen(profile, "a+");
    fread(buf_cutting(tmp_buf, sizeof(cs_id)), sizeof(cs_id), 1, profile_fd);
    fread(&axes_count, sizeof(axes_count), 1, profile_fd);
    fread(&vals_count, sizeof(vals_count), 1, profile_fd);
    fclose(profile_fd);


    long timestamp = now_microseconds();


    // Create a coordinate system object by MemAllocMng.
    MemAllocMng *space_mam = MemAllocMng_new();
    CoordinateSystem *coorsys = coosys_new(cs_id, axes_count, space_mam);


    log_print(">>> RELOAD_SPACE { 1 } %lf\n", (now_microseconds() - timestamp) / 1000.0);
    timestamp = now_microseconds();


    MemAllocMng *temp_mam = MemAllocMng_new();


    unsigned long vector_size = (sizeof(double) + sizeof(char)) * vals_count;


    for (int i = 0; i < axes_count; i++) {
        AxisBuildAssist *aba = mam_alloc(sizeof(AxisBuildAssist), OBJ_TYPE__AxisBuildAssist, temp_mam, 0);
        aba->leaf_scales_rbt = rbt_create("Scale *", scal_cmp, scal__destory, SPEC_MAM, temp_mam);
        ((Axis *)cs_get_axis(coorsys, i))->assist = aba;
    }


    FILE *data_fd = fopen(data_file, "a+");
    while (1)
    {
        for (int i = 0; i < axes_count; i++) {
            buf_clear(tmp_buf);
            // At the same time, hang the scale objects on the corresponding axis in the coordinate system object.
            Scale *sample = buf_cutting(tmp_buf, sizeof(Scale));
            if (fread(&(sample->fragments_len), sizeof(sample->fragments_len), 1, data_fd) < 1)
                goto has_loaded_scales;
            sample->fragments = buf_cutting(tmp_buf, sample->fragments_len * sizeof(md_gid));
            fread(sample->fragments, sample->fragments_len * sizeof(md_gid), 1, data_fd);

            Axis *axis = cs_get_axis(coorsys, i);
            if (rbt__find(axis->assist->leaf_scales_rbt, sample) == NULL)
            {
                Scale *scale = mam_alloc(sizeof(Scale), OBJ_TYPE__Scale, temp_mam, 0);
                scale->fragments_len = sample->fragments_len;
                scale->fragments = mam_alloc(scale->fragments_len * sizeof(md_gid), OBJ_TYPE__RAW_BYTES, temp_mam, 0);
                memcpy(scale->fragments, sample->fragments, scale->fragments_len * sizeof(md_gid));
                rbt_add(axis->assist->leaf_scales_rbt, scale);
            }
        }

        // skip some bytes
        buf_clear(tmp_buf);
        fread(buf_cutting(tmp_buf, vector_size), vector_size, 1, data_fd);
    }

has_loaded_scales:

    fclose(data_fd);

    // Calculate the actual size of the multidimensional array corresponding to the coordinate system.
    unsigned long space_capacity = 1;

    for (int i = 0; i < axes_count; i++) {
        Axis *axis = cs_get_axis(coorsys, i);
        rbt__reordering(axis->assist->leaf_scales_rbt);
        axis->scales_count = rbt__size(axis->assist->leaf_scales_rbt);

        // TODO FIXME
        // FIXME May exceed the maximum value of the long data type, if so, you need to customize the data type.
        space_capacity *= axis->scales_count;
    }


    {
        Axis *ax_n, *ax = als_get(coorsys->axes, axes_count - 1);
        ax->leaf_scale_offset = 1;
        for (int i = axes_count - 2; i >= 0; i--)
        {
            ax = als_get(coorsys->axes, i);
            ax_n = als_get(coorsys->axes, i + 1);
            ax->leaf_scale_offset = rbt__size(ax_n->assist->leaf_scales_rbt) * ax_n->leaf_scale_offset;
        }
    }


    log_print(">>> RELOAD_SPACE { 2 } %lf\n", (now_microseconds() - timestamp) / 1000.0);
    timestamp = now_microseconds();


    size_t space_partition_count = space_capacity / SPACE_DEF_PARTITION_SPAN_MIN;
    if (space_partition_count == 0)
    {
        space_partition_count = 1;
    }
    else if (space_capacity % SPACE_DEF_PARTITION_SPAN_MIN)
    {
        ++space_partition_count;
    }


    // Creates a new logical multidimensional array object by MemAllocMng.
    MeasureSpace *space = space_new(cs_id, space_partition_count, SPACE_DEF_PARTITION_SPAN_MIN, vals_count, space_mam);


    log_print(">>> RELOAD_SPACE { 3 } %lf\n", (now_microseconds() - timestamp) / 1000.0);
    timestamp = now_microseconds();


    unsigned long *loaded_pmea_count = mam_alloc(sizeof(unsigned long) * space_partition_count, OBJ_TYPE__RAW_BYTES, temp_mam, 0);
    char **cell_cursors = mam_alloc(sizeof(char *) * space_partition_count, OBJ_TYPE__RAW_BYTES, temp_mam, 0);

    RedBlackTree **tmp_rbt_hs = mam_alloc(sizeof(RedBlackTree *) * space_partition_count, OBJ_TYPE__RAW_BYTES, temp_mam, 0);
    for (size_t i=0; i<space_partition_count ;i++)
        tmp_rbt_hs[i] = rbt_create("*cell", cell_cmp, _cell__destory, SPEC_MAM, MemAllocMng_new());


    // Traverse the data file and insert all measure data into the logical multidimensional array.
    data_fd = fopen(data_file, "a+");

    while (1)
    {
        unsigned long vector_offset = 0;
        for (int i = 0; i < axes_count; i++)
        {
            buf_clear(tmp_buf);
            Scale *sample = buf_cutting(tmp_buf, sizeof(Scale));
            if (fread(&(sample->fragments_len), sizeof(int), 1, data_fd) < 1)
                goto loaded_vectors_done;

            sample->fragments = buf_cutting(tmp_buf, sample->fragments_len * sizeof(md_gid));
            fread(sample->fragments, sizeof(md_gid), sample->fragments_len, data_fd);
            Axis *axis = cs_get_axis(coorsys, i);
            vector_offset += rbt__find(axis->assist->leaf_scales_rbt, sample)->index * axis->leaf_scale_offset;
        }

        unsigned long segment_number = vector_offset / space->segment_scope;
        if (loaded_pmea_count[segment_number] % BYTES_ALIGNMENT == 0)
        {
            MemAllocMng *seg_mam = obj_mam(tmp_rbt_hs[segment_number]);
            cell_cursors[segment_number] = mam_hlloc(seg_mam, (sizeof(vector_offset) + vector_size) * BYTES_ALIGNMENT);
        }
        void *cell = cell_cursors[segment_number] + (sizeof(vector_offset) + vector_size) * (loaded_pmea_count[segment_number] % BYTES_ALIGNMENT);
        *((unsigned long *)cell) = vector_offset;
        fread(cell + sizeof(vector_offset), vector_size, 1, data_fd);
        rbt_add(tmp_rbt_hs[segment_number], cell);
        ++(loaded_pmea_count[segment_number]);
        if (loaded_pmea_count[segment_number] % 1000000 == 0)
            log_print("::::::::::::::::::: load_meval_count[%lu] = %lu\n", segment_number, loaded_pmea_count[segment_number]);
    }

loaded_vectors_done:

    buf_release(tmp_buf);

    fclose(data_fd);


    log_print(">>> RELOAD_SPACE { 4 } %lf\n", (now_microseconds() - timestamp) / 1000.0);
    timestamp = now_microseconds();


    space_plan(space, tmp_rbt_hs);


    {
        for (int i = 0; i < axes_count; i++)
        {
            Axis *ax = als_get(coorsys->axes, i);
            RedBlackTree *tree = ax->assist->leaf_scales_rbt;
            rbt__scan_do(tree, &(ax->max_path_len), __set_ax_max_path_len);
            ax->assist->scales_table = mam_alloc(rbt__size(tree) * ax->max_path_len * sizeof(md_gid), OBJ_TYPE__RAW_BYTES, temp_mam, 0);
            rbt__scan_do(tree, ax, __Axis_build_index);
        }
    }


    {
        for (int i = 0; i < axes_count; i++)
        {
            Axis *axis = als_get(coorsys->axes, i);

            /* TODO
            * When the red-black tree adds elements, if there is another equal element, the red-black tree will not do anything,
            * which may cause some problems. Here, a new red-black tree is temporarily created, and this place needs to be
            * performed in the follow-up. Revise.
            */
            axis->sor_idx_tree = rbt_create("ScaleOffsetRange *", ScaleOffsetRange_cmp, ScaleOffsetRange_destory, SPEC_MAM, space_mam);
            int tree_sz = rbt__size(axis->assist->leaf_scales_rbt);
            ScaleOffsetRange *sor = NULL;
            md_gid id, prev_id = 0;

            int row, col;
            for (col = 0; col < axis->max_path_len; col++)
            {
                for (row = 0; row < tree_sz; row++)
                {
                    id = ((md_gid *)axis->assist->scales_table)[row * axis->max_path_len + col];

                    if (sor == NULL || id != prev_id)
                    {
                        sor = mam_alloc(sizeof(ScaleOffsetRange), OBJ_TYPE__ScaleOffsetRange, space_mam, 0);
                        sor->gid = id;
                        sor->offset = axis->leaf_scale_offset;
                        sor->end_position = sor->start_position = row;
                        rbt_add(axis->sor_idx_tree, sor);
                    }
                    else
                    {
                        sor->end_position = row;
                    }

                    prev_id = id;
                }
            }
            rbt__reordering(axis->sor_idx_tree);
            axis->sor_idx_tree_size = rbt__size(axis->sor_idx_tree);
        }
    }


    for (int i = 0; i < axes_count; i++)
        ((Axis *)cs_get_axis(coorsys, i))->assist = NULL;


    mam_reset(temp_mam);
    obj_release(temp_mam->current_block);
    obj_release(temp_mam);


    log_print(">>> RELOAD_SPACE { 5 } %lf\n", (now_microseconds() - timestamp) / 1000.0);


    *_cs = coorsys;
    *ms_ = space;

}

void reload_space(unsigned long cs_id)
{
    assert(strlen(olap_env.OLAP_HOME) < 200);

    char profile[256], data_file[256], coorsys_mirror[256], data_mirror[256];
    memset(profile, 0, 256);
    memset(data_file, 0, 256);
    memset(coorsys_mirror, 0, 256);
    memset(data_mirror, 0, 256);
    sprintf(profile, "%s/data/profile-%lu", olap_env.OLAP_HOME, cs_id);
    sprintf(data_file, "%s/data/data-%lu", olap_env.OLAP_HOME, cs_id);
    sprintf(coorsys_mirror, "%s/data/mirror-coorsys-%lu", olap_env.OLAP_HOME, cs_id);
    sprintf(data_mirror, "%s/data/mirror-data-%lu", olap_env.OLAP_HOME, cs_id);


    CoordinateSystem *coorsys = NULL;
    MeasureSpace *space = NULL;


    if (access(coorsys_mirror, F_OK) == 0 && access(data_mirror, F_OK) == 0) {
        load_mirror(coorsys_mirror, data_mirror, cs_id, &coorsys, &space);
    } else if (access(profile, F_OK) == 0 && access(data_file, F_OK) == 0) {
        load_space(profile, data_file, cs_id, &coorsys, &space);
    } else {
        return;
    }

    space_unload(cs_id);
    als_add(coor_sys_ls, coorsys);
    als_add(space_ls, space);
}

CoordinateSystem *coosys_new(unsigned long id, int axes_count, MemAllocMng *mam)
{

    CoordinateSystem *cs = mam_alloc(sizeof(CoordinateSystem), OBJ_TYPE__CoordinateSystem, mam, 1);
    cs->id = id;
    cs->axes = als_new(axes_count, "Axis *", SPEC_MAM, mam);

    for (int i = 0; i < axes_count; i++)
    {
        Axis *axis = ax_create(mam);
        als_add(cs->axes, axis);
    }

    return cs;
}

Axis *ax_create(MemAllocMng *mam)
{
    Axis *ax = mam_alloc(sizeof(Axis), OBJ_TYPE__Axis, mam, 0);

    // ax->rbtree = rbt_create("struct _axis_scale *", scal_cmp, scal__destory, SPEC_MAM, mam);

    ax->sor_idx_tree = rbt_create("ScaleOffsetRange *", ScaleOffsetRange_cmp, ScaleOffsetRange_destory, SPEC_MAM, mam);

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

void cs_add_axis(CoordinateSystem *cs, Axis *axis)
{
    als_add(cs->axes, axis);
}

int scal_cmp(void *_one, void *_other)
{
    Scale *one = (Scale *)_one, *other = (Scale *)_other;

    int i, len_c = one->fragments_len < other->fragments_len ? one->fragments_len : other->fragments_len;
    __uint64_t *f = one->fragments;
    __uint64_t *o_f = other->fragments;
    for (i = 0; i < len_c; i++)
    {
        if (*(o_f + i) < *(f + i))
            return -1;
        if (*(o_f + i) > *(f + i))
            return 1;
    }
    return one->fragments_len < other->fragments_len ? 1 : (one->fragments_len > other->fragments_len ? -1 : 0);
}

void space_unload(__uint64_t id)
{
    for (int i = 0; i < als_size(coor_sys_ls); i++)
    {
        CoordinateSystem *cs = als_get(coor_sys_ls, i);
        if (cs->id == id)
        {
            als_rm_index(coor_sys_ls, i);
            break;
        }
    }

    for (int i = 0; i < als_size(space_ls); i++)
    {
        MeasureSpace *ms = als_get(space_ls, i);
        if (ms->id == id)
        {
            als_rm_index(space_ls, i);

            short type;
            enum_oms strat;
            MemAllocMng *mam;
            obj_info(ms, &type, &strat, &mam);

            mam_reset(mam);
            obj_release(mam->current_block);
            obj_release(mam);

            break;
        }
    }
}

int ax_size(Axis *axis)
{
    return rbt__size(axis->assist->leaf_scales_rbt);
}

MeasureSpace *space_new(unsigned long id, size_t segment_count, size_t segment_scope, int cell_vals_count, MemAllocMng *mam)
{

    MeasureSpace *s = mam_alloc(sizeof(MeasureSpace), OBJ_TYPE__MeasureSpace, mam, 1);
    s->id = id;
    s->cell_vals_count = cell_vals_count;
    s->segment_count = segment_count;
    s->segment_scope = segment_scope;
    // s->tree_ls_h = mam_alloc(sizeof(RedBlackTree *) * segment_count, OBJ_TYPE__RedBlackTree, mam, 0);
    s->data_ls_h = mam_alloc(sizeof(void *) * segment_count, OBJ_TYPE__RAW_BYTES, mam, 0);
    s->data_lens = mam_alloc(sizeof(unsigned long) * segment_count, OBJ_TYPE__RAW_BYTES, mam, 0);

    // for (int i = 0; i < segment_count; i++)
    //     s->tree_ls_h[i] = rbt_create("*cell", cell_cmp, _cell__destory, SPEC_MAM, mam);

    return s;
}

void *build_space_measure(RBNode *node, void *callback_params)
{
    int cell_size = *((int *)callback_params) + sizeof(long); // bytes count
    void *block_addr = *((void **)(callback_params + sizeof(int)));
    memcpy(block_addr + cell_size * (node->index), node->obj, cell_size);
}

void space_plan(MeasureSpace *space, RedBlackTree **tmp_rbt_hs)
{
    MemAllocMng *space_mam = obj_mam(space);
    int cell_size = space->cell_vals_count * (sizeof(double) + sizeof(char));
    for (int i = 0; i < space->segment_count; i++)
    {
        RedBlackTree *tree = tmp_rbt_hs[i];
        rbt__reordering(tree);
        unsigned int actual_cells_sz = rbt__size(tree);
        space->data_lens[i] = actual_cells_sz;
        int posi_cell_sz = (sizeof(unsigned long) + cell_size);
        space->data_ls_h[i] = mam_alloc(actual_cells_sz * posi_cell_sz, OBJ_TYPE__RAW_BYTES, space_mam, 0);

        char callback_params[sizeof(int) + sizeof(void *)];
        *((int *)callback_params) = cell_size;
        memcpy(&(callback_params[sizeof(int)]), space->data_ls_h + i, sizeof(void *));
        rbt__scan_do(tree, callback_params, build_space_measure);

        MemAllocMng *tmp_seg_mam = obj_mam(tree);
        mam_reset(tmp_seg_mam);
        obj_release(tmp_seg_mam->current_block);
        obj_release(tmp_seg_mam);
    }
}

__uint64_t cs_axis_span(CoordinateSystem *cs, int axis_order)
{
    unsigned int axes_count = als_size(cs->axes);
    unsigned long span = 1;
    int i;
    for (i = axes_count - 1; i > axis_order; i--)
    {
        Axis *x = cs_get_axis(cs, i);
        span *= rbt__size(x->assist->leaf_scales_rbt);
    }
    return span;
}

void space_add_measure(MeasureSpace *space, __uint64_t measure_position, void *cell)
{
    // rbt_add(space->tree_ls_h[measure_position / space->segment_scope], cell);
}

void Scale_print(Scale *s)
{
    log_print("Scale <%p> [ fragments_len = %d ] ", s, s->fragments_len);
    int i;
    for (i = 0; i < s->fragments_len; i++)
    {
        log_print("  %lu", s->fragments[i]);
    }
    log_print("\n");
}

void space__destory(MeasureSpace *s)
{
    size_t i;
    for (i = 0; i < s->segment_count; i++)
    {
        // if (s->tree_ls_h[i])
        //     rbt__destory(s->tree_ls_h[i]);

        // if (s->data_ls_h[i])
        //     _release_mem_(s->data_ls_h[i]);
    }

    // _release_mem_(s->tree_ls_h);

    // _release_mem_(s->data_ls_h);

    // _release_mem_(s);
}

/*
 * @return ArrayList<GridData *>
 */
ArrayList *vce_vactors_values(MDContext *md_ctx, MddTuple **tuples_matrix_h, unsigned long v_len)
{
    /**
     * todo
     * If the current node is running in master mode, you need to send the aggregate query request to the worker node,
     * wait at the semaphore, and wait until the worker node is woken up after all executions are completed.
     */
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
            //     log_print("[ info ] - This member role represents a calculation formula member.\n");
        }
    }

    // double *result = mam_alloc(v_len * sizeof(double), OBJ_TYPE__RAW_BYTES, NULL, 0);
    // *null_flags = mam_alloc(v_len * sizeof(char), OBJ_TYPE__RAW_BYTES, NULL, 0);

    // GridData tmp;
    GridData *gd_arr = mam_alloc(v_len * sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);
    ArrayList *gd_list = als_new(v_len, "GridData *", THREAD_MAM, NULL);

    for (i = 0; i < v_len; i++)
    {
        do_calculate_measure_value(md_ctx, cube, tuples_matrix_h[i], gd_arr + i);
        als_add(gd_list, gd_arr + i);
        // if (tmp.null_flag)
        //     (*null_flags)[i] = 1;
        // else
        //     result[i] = tmp.val;
    }

    return gd_list;
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
    memcpy(ax->assist->scales_table + node->index * ax->max_path_len * sizeof(md_gid), scale->fragments, scale->fragments_len * sizeof(md_gid));
    return NULL;
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
            
            Member *def_mbr = get_default_dimension_member(cube, find_dim_by_gid(mr->dim_role->dim_gid));
            MddMemberRole *def_mrole = mdd_mr__create(def_mbr, mr->dim_role);
            MddTuple *temp_tuple = mdd_tp__create();
            mdd_tp__add_mbrole(temp_tuple, def_mrole);
            MddTuple *new_tuple = tuple__merge(tuple, temp_tuple);

            Expression_evaluate(md_ctx, exp, cube, new_tuple, grid_data);
            return;
        }
    }

    ArrayList_sort(tuple->mr_ls, MddMemberRole_cmp);

    EuclidConfig *cfg = get_cfg();

    if (cfg->mode == MODE_MASTER)
    {
        ArrayList *one_tp_ls = als_new(1, "MddTuple *", THREAD_MAM, NULL);
        als_add(one_tp_ls, tuple);

        double *mea_val__;
        char *null_flag__;
        unsigned long len__;

        dispatchAggregateMeasure(/*md_ctx,*/ cube, one_tp_ls, &mea_val__, &null_flag__, &len__);

        grid_data->val = *mea_val__;
        grid_data->null_flag = *null_flag__;

        return;
    }

    int coor_count = als_size(coor_sys_ls);

    CoordinateSystem *coor = NULL;

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

    if (coor == NULL)
    {
        MemAllocMng *thrd_mam = MemAllocMng_current_thread_mam();
        thrd_mam->exception_desc = "No measure data of cube.";
        longjmp(thrd_mam->excep_ctx_env, -1);
    }

    ArrayList *sor_ls = als_new(64, "ScaleOffsetRange *", THREAD_MAM, NULL);

    MddMemberRole *measure_mr;

    for (i = 0; i < sz; i++)
    {
        MddMemberRole *mr = als_get(tuple->mr_ls, i);
        DimensionRole *dr = mr->dim_role;

        // continue measure member role

        if (dr->bin_attr & DR_MEASURE_MASK)
        // if (dr == NULL)
        {
            measure_mr = mr;
            continue;
        }

        ScaleOffsetRange *key = mam_alloc(sizeof(ScaleOffsetRange), OBJ_TYPE__ScaleOffsetRange, NULL, 0);
        if (mr->member->lv == 0) {
            key->gid = 0;
            als_add(sor_ls, key);
        } else {
            Axis *ax = cs_get_axis(coor, i);
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

    // Each time the cell is queried, it is executed once.
    // sor_ls observed, dimensions that are all summarized are represented by a 0
    _do_calculate_measure_value(
        // md_ctx,
        space, sor_ls, 0, 0, grid_data, mea_val_idx);
}

static void _do_calculate_measure_value(
    // MDContext *md_ctx,
    MeasureSpace *space, ArrayList *sor_ls, int deep, unsigned long offset, GridData *grid_data, int mea_val_idx)
{
    // Observe the sor_ls and make a range summary from the last position that is not 0
    // If all are 0s, a full rollup is performed.
    // This operation should not be too CPU-intensive.
    int dynamic_index_scope = als_size(sor_ls);
    for (int i=als_size(sor_ls)-1;i>=0;i--) {
        ScaleOffsetRange *scor = als_get(sor_ls, i);
        if (scor->gid) {
            break;
        } else {
            dynamic_index_scope--;
        }
    }

    if (dynamic_index_scope == 0) {
        // Fully summarized
        calculates_the_full_summary_value(space, grid_data, mea_val_idx);
        return;
    }

    agg_based_on_dynamic_sparse_idx(space, sor_ls, dynamic_index_scope, deep, offset, grid_data, mea_val_idx);
}

void ScaleOffsetRange_print(ScaleOffsetRange *sor)
{
    log_print("[ ScaleOffsetRange %p ] gid = %lu, position [ %lu, %lu ]\n", sor, sor->gid, sor->start_position, sor->end_position);
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
    // log_print("[ func - ScaleOffsetRange_destory ] This function has not been implemented yet.\n");
    return NULL;
}

static void calculates_the_full_summary_value(MeasureSpace *space, GridData *gridData, int mea_val_idx) {
    gridData->type = GRIDDATA_TYPE_NUM;
    gridData->null_flag = 1;
    gridData->val = 0;
    int vlen = sizeof(unsigned long) + ( sizeof(double) + sizeof(char) ) * space->cell_vals_count;
    for (size_t i=0;i<space->segment_count;i++) {
        char *data_segment = space->data_ls_h[i];
        for (unsigned long j=0;j<space->data_lens[i];j++) {
            char *val_addr = data_segment + vlen * j + sizeof(unsigned long) + ( sizeof(double) + sizeof(char) ) * mea_val_idx;
            if (*(char *)(val_addr + sizeof(double)) == 0) {
                gridData->null_flag = 0;
                gridData->val += *(double *)val_addr;
            }
        }
    }
}

static void summarize_space_scope(MeasureSpace *space, unsigned long start_offset, unsigned long end_offset, int mea_val_idx, GridData *gridData) {

    if (start_offset == end_offset) {
        MeasureSpace_coordinate_intersection_value(space, start_offset, mea_val_idx, gridData);
        return;
    }

    assert(end_offset > start_offset);

    unsigned long staing_seg_num = start_offset / space->segment_scope;
    unsigned long ending_seg_num = end_offset / space->segment_scope;

    assert(ending_seg_num >= staing_seg_num);

    int a_cell_bytes_count = space->cell_vals_count * (sizeof(double) + sizeof(char)) + sizeof(unsigned long);

    /**
     * Head position
     * If the header position precedes the start point of the block, the header position is equal to the start pointer.
     * If the header position is after the end of the block, the header position is equal to NULL.
     * If neither of the above, find the first pointer that is not less than the coordinate index of the header position,
     * and the head position is equal to it.
     */
    char *first_block = space->data_ls_h[staing_seg_num];
    unsigned long first_cells_count = space->data_lens[staing_seg_num];

    char *f_tail_addr = first_block + (first_cells_count - 1) * a_cell_bytes_count;

    unsigned long f_head_offset = *(unsigned long *)first_block;
    unsigned long f_tail_offset = *(unsigned long *)f_tail_addr;

    char *start_addr = NULL;

    if (start_offset < f_head_offset) {
        start_addr = first_block;
    } else if (start_offset > f_tail_offset) {
        start_addr = NULL;
    } else {
        unsigned long range_start = 0;
        unsigned long range_end = first_cells_count - 1;

        while (1) {
            if (range_end - range_start > 1) {
                unsigned long range_mid = (range_start + range_end) / 2;
                char *mid_addr = first_block + range_mid * a_cell_bytes_count;
                unsigned long mid_offset = *((unsigned long *)mid_addr);
                if (mid_offset == start_offset)
                {
                    start_addr = mid_addr;
                    break;
                } else if (start_offset < mid_offset) {
                    range_end = range_mid;
                } else { // start_offset > mid_offset
                    range_start = range_mid;
                }
            } else {
                unsigned long range_start_offset = *((unsigned long *)(first_block + range_start * a_cell_bytes_count));
                if (range_start_offset >= start_offset) {
                    start_addr = first_block + range_start * a_cell_bytes_count;
                } else {
                    start_addr = first_block + range_end * a_cell_bytes_count;
                }
                break;
            }
        }
    }

    /**
     * Tail position
     * If the tail position is after the end of the data block, the tail position is equal to the end pointer.
     * If the tail position precedes the start of the block, the tail position is equal to NULL.
     * If neither of the above, find the last pointer that is not greater than the index of the tail position coordinates,
     * and the tail position is equal to it.
     */
    char *last_block = space->data_ls_h[ending_seg_num];
    unsigned long last_cells_count = space->data_lens[ending_seg_num];

    char *l_tail_addr = last_block + (last_cells_count - 1) * a_cell_bytes_count;

    unsigned long l_head_offset = *(unsigned long *)last_block;
    unsigned long l_tail_offset = *(unsigned long *)l_tail_addr;
    char *end_addr = NULL;

    if (end_offset > l_tail_offset) {
        end_addr = l_tail_addr;
    } else if (end_offset < l_head_offset) {
        end_addr = NULL;
    } else {
        unsigned long range_start = 0;
        unsigned long range_end = last_cells_count - 1;

        while (1) {
            if (range_end - range_start > 1) {
                unsigned long range_mid = (range_start + range_end) / 2;
                char *mid_addr = last_block + range_mid * a_cell_bytes_count;
                unsigned long mid_offset = *((unsigned long *)mid_addr);

                if (mid_offset == end_offset)
                {
                    end_addr = mid_addr;
                    break;
                } else if (end_offset < mid_offset) {
                    range_end = range_mid;
                } else { // end_offset > mid_offset
                    range_start = range_mid;
                }
            } else {
                unsigned long range_end_offset = *((unsigned long *)(last_block + range_end * a_cell_bytes_count));
                if (range_end_offset <= end_offset) {
                    end_addr = last_block + range_end * a_cell_bytes_count;
                } else {
                    end_addr = last_block + range_start * a_cell_bytes_count;
                }
                break;
            }
        }
    }

    char *cursor = NULL;

    if (staing_seg_num == ending_seg_num) {
        if (start_addr && end_addr) {
            cursor = start_addr;
            while (cursor <= end_addr) {
                char nullflag = *(char *)(cursor + sizeof(unsigned long) + mea_val_idx * (sizeof(double) + sizeof(char)) + sizeof(double));
                if (!nullflag) {
                    double meaval = *(double *)(cursor + sizeof(unsigned long) + mea_val_idx * (sizeof(double) + sizeof(char)));
                    gridData->null_flag = 0;
                    gridData->val += meaval;
                }
                cursor += a_cell_bytes_count;
            }
        } else {
            gridData->null_flag = 1;
        }
    } else {
        // If the start_index is not NULL, summarize the data block of start
        if (start_addr) {
            cursor = start_addr;
            while (cursor <= f_tail_addr) {
                char nullflag = *(char *)(cursor + sizeof(unsigned long) + mea_val_idx * (sizeof(double) + sizeof(char)) + sizeof(double));
                if (!nullflag) {
                    double meaval = *(double *)(cursor + sizeof(unsigned long) + mea_val_idx * (sizeof(double) + sizeof(char)));
                    gridData->null_flag = 0;
                    gridData->val += meaval;
                }
                cursor += a_cell_bytes_count;
            }
        }

        // Summarize blocks of data between start and end.
        // assert(ending_seg_num >= staing_seg_num);
        for (unsigned long i = staing_seg_num + 1; i < ending_seg_num; i++) {
            char *mid_block = space->data_ls_h[i];
            unsigned long midblk_cells_count = space->data_lens[i];

            char *midblk_tail_addr = mid_block + (midblk_cells_count - 1) * a_cell_bytes_count;

            unsigned long midblk_head_idx = *(unsigned long *)mid_block;
            unsigned long midblk_tail_idx = *(unsigned long *)midblk_tail_addr;
            cursor = mid_block;
            while (cursor <= midblk_tail_addr) {
                char nullflag = *(char *)(cursor + sizeof(unsigned long) + mea_val_idx * (sizeof(double) + sizeof(char)) + sizeof(double));
                if (!nullflag) {
                    double meaval = *(double *)(cursor + sizeof(unsigned long) + mea_val_idx * (sizeof(double) + sizeof(char)));
                    gridData->null_flag = 0;
                    gridData->val += meaval;
                }
                cursor += a_cell_bytes_count;
            }
        }

        // If the end_index is not NULL, summarize the end data block.
        if (end_addr) {
            cursor = last_block;
            while (cursor <= end_addr) {
                char nullflag = *(char *)(cursor + sizeof(unsigned long) + mea_val_idx * (sizeof(double) + sizeof(char)) + sizeof(double));
                if (!nullflag) {
                    double meaval = *(double *)(cursor + sizeof(unsigned long) + mea_val_idx * (sizeof(double) + sizeof(char)));
                    gridData->null_flag = 0;
                    gridData->val += meaval;
                }
                cursor += a_cell_bytes_count;
            }
        }
    }
}

static void MeasureSpace_coordinate_intersection_value(MeasureSpace *space, unsigned long index, int mea_val_idx, GridData *gridData)
{
    gridData->null_flag = 0; // default not null

    unsigned long segment_num = index / space->segment_scope;
    // if (segment_num != 0 && !(index % space->segment_scope))
    //     --segment_num;

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

void dispatchAggregateMeasure(/*MDContext *md_context,*/ Cube *cube, ArrayList *direct_vectors, double **_measures_, char **_null_flags_, unsigned long *_len_)
{

    unsigned long task_group_code = gen_md_gid();

    // Cube *cube = select_def__get_cube(md_context->select_def);
    int dr_count = als_size(cube->dim_role_ls);

    unsigned int size = als_size(direct_vectors);

    unsigned int pkg_size = 4 + 2 + 8 + 8 + 4 + 4 + 8 + (sizeof(md_gid) * dr_count + sizeof(int)) * size;
    ByteBuf *buff = buf__alloc(pkg_size);

    *((unsigned int *)buf_cutting(buff, sizeof(int))) = pkg_size;
    *((unsigned short *)buf_cutting(buff, sizeof(short))) = INTENT__VECTOR_AGGREGATION;
    *((md_gid *)buf_cutting(buff, sizeof(md_gid))) = cube->gid;
    *((unsigned long *)buf_cutting(buff, sizeof(long))) = task_group_code;
    buf_cutting(buff, sizeof(int)); // skip the max task group number
    buf_cutting(buff, sizeof(int)); // skip the task group number
    *((long *)buf_cutting(buff, sizeof(long))) = size;

    for (int i = 0; i < size; i++)
    {
        MddTuple *tuple = als_get(direct_vectors, i);
        ArrayList_sort(tuple->mr_ls, MddMemberRole_cmp);

        MddMemberRole *measure_mr = NULL;

        int count = als_size(tuple->mr_ls);
        for (int j = 0; j < count; j++)
        {
            MddMemberRole *mr = als_get(tuple->mr_ls, j);
            DimensionRole *dr = mr->dim_role;

            if (dr->bin_attr & DR_MEASURE_MASK)
            {
                measure_mr = mr;
                continue;
            }

            // if member is a root, then use zero to replace its gid.
            *((md_gid *)buf_cutting(buff, sizeof(md_gid))) = mr->member->p_gid ? mr->member->gid : 0;
        }

        // int mea_val_idx = 0;
        Member *measure_m = measure_mr->member;
        for (int j = 0; j < als_size(cube->measure_mbrs); j++)
        {
            Member *_m = (Member *)als_get(cube->measure_mbrs, j);
            if (measure_m->gid == _m->gid)
            {
                // mea_val_idx = j;
                buf_append_int(buff, j);
                break;
            }
        }
    }

    ArrayList *node_list = worker_nodes();

    size_t buf_sz = buf_size(buff);

    // buf_clear(buf);
    // *((int *) buf_cutting(buf, 4+2+8+8)) = als_size(node_list);
    buf_set_cursor(buff, 4 + 2 + 8 + 8);

    int max_task_grp_num = als_size(node_list);

    buf_append_int(buff, max_task_grp_num);

    sem_t semt;
    put_agg_task_group(task_group_code, max_task_grp_num, &semt);

    sem_init(&semt, 0, 0);

    for (int i = 0; i < als_size(node_list); i++)
    {
        SockIntentThread *node_sit = als_get(node_list, i);

        *((int *)buf_cutting(buff, sizeof(int))) = i;

        ssize_t rscode = send(node_sit->sock_fd, buf_starting(buff), buf_sz, 0);

        buf_set_cursor(buff, 4 + 2 + 8 + 8);
    }

    for (int i = 0; i < max_task_grp_num; i++)
    {
        sem_wait(&semt);
    }

    sem_destroy(&semt);

    double *measure_vals = NULL;
    char *null_flags = NULL;
    int vals_size;
    agg_task_group_result(task_group_code, &measure_vals, &null_flags, &vals_size);

    assert(((measure_vals == NULL) && (null_flags == NULL)) || ((measure_vals != NULL) && (null_flags != NULL)));

    if (measure_vals)
    {
        *_measures_ = mam_alloc(sizeof(double) * vals_size, OBJ_TYPE__RAW_BYTES, NULL, 0);
        *_null_flags_ = mam_alloc(sizeof(char) * vals_size, OBJ_TYPE__RAW_BYTES, NULL, 0);
        *_len_ = vals_size;

        memcpy(*_measures_, measure_vals, vals_size * sizeof(double));
        memcpy(*_null_flags_, null_flags, vals_size * sizeof(char));

        obj_release(measure_vals);
        obj_release(null_flags);
    }
    else
    {
        *_measures_ = mam_alloc(sizeof(double) * als_size(direct_vectors), OBJ_TYPE__RAW_BYTES, NULL, 0);
        *_null_flags_ = mam_alloc(sizeof(char) * als_size(direct_vectors), OBJ_TYPE__RAW_BYTES, NULL, 0);
        memset(*_null_flags_, 1, sizeof(char) * als_size(direct_vectors));
        *_len_ = als_size(direct_vectors);

        // memset(*_null_flags_, 1, als_size(direct_vectors));
    }

    buf_release(buff);
}

void MeasureSpace_print(MeasureSpace *space)
{
    log_print(">>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    log_print("id              - %lu\n", space->id);
    log_print("segment_count   - %lu\n", space->segment_count);
    log_print("segment_scope   - %lu\n", space->segment_scope);
    log_print("cell_vals_count - %d\n", space->cell_vals_count);

    int cell_len = sizeof(long) + space->cell_vals_count * (sizeof(double) + sizeof(char));

    int i;
    for (i = 0; i < space->segment_count; i++)
    {
        log_print("\n");

        log_print("%lu\n", space->data_lens[i]);
        int j;
        for (j = 0; j < space->data_lens[i]; j++)
        {
            void *cell = space->data_ls_h[i] + j * cell_len;
            log_print("% 12lu  >  ", *((unsigned long *)cell));
            cell += sizeof(long);
            int k;
            for (k = 0; k < space->cell_vals_count; k++)
            {
                log_print("% 20lf - ", *((double *)(cell + k * (sizeof(double) + sizeof(char)))));
                log_print("%d    ", *((char *)(cell + k * (sizeof(double) + sizeof(char)) + sizeof(double))));
            }
            log_print("\n");
        }

        log_print("\n");
    }

    log_print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>>>>>>>>>\n");
    log_print(">>>>>>>>\n");
}

// Axis(struct _coordinate_axis) functions
Scale *ax_find_scale(Axis *axis, Scale *sample)
{
    RBNode *node = rbt__find(axis->assist->leaf_scales_rbt, sample);
    return node ? node->obj : NULL;
}

// Scale(struct _axis_scale) functions
void scal_init(Scale *scale)
{
    scale->fragments_len = 0;
    scale->fragments = NULL;
}

ArrayList *worker_aggregate_measure(EuclidCommand *ec)
{

    char *payload = ec->bytes;
    unsigned int idx = 4 + 2;

    md_gid cube_gid = *((md_gid *)(payload + idx));
    CoordinateSystem *cs = NULL;
    for (int i = 0; i < als_size(coor_sys_ls); i++)
    {
        cs = als_get(coor_sys_ls, i);
        if (cs->id == cube_gid)
            break;
        cs = NULL;
    }

    if (cs == NULL)
        return NULL;

    MeasureSpace *space;
    for (int i = 0; i < als_size(space_ls); i++)
    {
        space = (MeasureSpace *)als_get(space_ls, i);
        if (space->id == cube_gid)
            break;
        else
            space = NULL;
    }

    assert(cs != NULL && space != NULL);

    idx = 4 + 2 + 8 + 8 + 4 + 4;

    unsigned long quantity_vectors = *((unsigned long *)(payload + idx));

    int ax_count = als_size(cs->axes);

    idx += sizeof(long);

    ScaleOffsetRange key;
    memset(&key, 0, sizeof(ScaleOffsetRange));

    MemAllocMng *thread_mam = MemAllocMng_current_thread_mam();
    ArrayList *grids = als_new((unsigned int)quantity_vectors, "GridData *", SPEC_MAM, thread_mam);

    for (int i = 0; i < quantity_vectors; i++)
    {
        ArrayList *sor_ls = als_new(64, "ScaleOffsetRange *", THREAD_MAM, NULL);

        char mark_null = 0;

        for (int j = 0; j < ax_count; j++)
        {

            Axis *ax = cs_get_axis(cs, j);

            md_gid member_gid = *((md_gid *)(payload + idx));
            idx += sizeof(md_gid);
            // log_print("\tmember_gid = %ld\n", member_gid);

            if (member_gid == 0) {
                als_add(sor_ls, mam_alloc(sizeof(ScaleOffsetRange), OBJ_TYPE__ScaleOffsetRange, NULL, 0));
                continue;
            }

            key.gid = member_gid;

            RBNode *node = rbt__find(ax->sor_idx_tree, &key);

            if (node == NULL)
            {
                mark_null = 1;
                // break;
                // als_add(grids, NULL);
            }
            else
            {
                ScaleOffsetRange *sor = (ScaleOffsetRange *)node->obj;
                als_add(sor_ls, sor);
            }

            // idx += sizeof(md_gid);
        }

        if (mark_null)
        {
            als_add(grids, NULL);
            idx += sizeof(int);
            continue;
        }

        int measure_idx = *((int *)(payload + idx));
        // log_print("--------measure_idx = %d\n", measure_idx);
        idx += sizeof(int);

        GridData *grid = mam_hlloc(thread_mam, sizeof(GridData));
        // memset(grid, 0, sizeof(GridData));

        grid->null_flag = 1; // measure value is default null

        _do_calculate_measure_value(space, sor_ls, 0, 0, grid, measure_idx);

        als_add(grids, grid);
    }

    return grids;
}

/**
 * The third parameter dyc_uidx_len which represents the dynamic union index length,
 * can be understood as the number of dimensions involved in the dynamic sparse dimension.
 */
static void agg_based_on_dynamic_sparse_idx
        (MeasureSpace *space, ArrayList *sor_ls, int dyc_uidx_len, int deep,
        unsigned long offset, GridData *grid_data, int mea_val_idx) {

    ScaleOffsetRange *sor = (ScaleOffsetRange *)als_get(sor_ls, deep);
    unsigned long _position;

    // It can be summarized on the current axis (the subsequent axes are all summarized).
    CoordinateSystem *coor = NULL;
    for (int i = 0; i < als_size(coor_sys_ls); i++)
    {
        coor = als_get(coor_sys_ls, i);
        if (coor->id == space->id) {
            break;
        }
        coor = NULL;
    }

    Axis *ax = cs_get_axis(coor, deep);

    if (deep != (dyc_uidx_len - 1)) {
        // Continue to recurse backwards.
        if (sor->gid) {
            for (_position = sor->start_position; _position <= sor->end_position; _position++) {
                agg_based_on_dynamic_sparse_idx(space, sor_ls, dyc_uidx_len, deep + 1, offset + _position * sor->offset, grid_data, mea_val_idx);
            }
        } else {
            for (_position = 0; _position < ax->scales_count; _position++) {
                agg_based_on_dynamic_sparse_idx(space, sor_ls, dyc_uidx_len, deep + 1, offset + _position * ax->leaf_scale_offset, grid_data, mea_val_idx);
            }
        }
        return;
    }

    GridData chip;
    memset(&chip, 0, sizeof(GridData));

    summarize_space_scope(space, 
        offset + sor->start_position * sor->offset, 
        offset + (sor->end_position + 1) * sor->offset - 1, 
        mea_val_idx, &chip);

    if (chip.null_flag == 0) {
        grid_data->null_flag = 0;
        grid_data->val += chip.val;
    }
}