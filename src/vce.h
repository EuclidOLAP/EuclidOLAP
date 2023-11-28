#ifndef EUCLID__VCE_H
#define EUCLID__VCE_H 1

#include "command.h"
#include "utils.h"
#include "rb-tree.h"
#include "mdd.h"
#include "mdx.h"

#define SPACE_DEF_PARTITION_COUNT 128
#define SPACE_DEF_PARTITION_SPAN_MIN 1000000000

typedef struct _coordinate_system
{
    __uint64_t id;
    ArrayList *axes;
} CoordinateSystem;

CoordinateSystem *coosys_new(unsigned long id, int axes_count, MemAllocMng *mam);

typedef struct _ax_build_assist_ {
    RedBlackTree *leaf_scales_rbt;
    char *scales_table;
} AxisBuildAssist;

typedef struct _coordinate_axis
{
    RedBlackTree *sor_idx_tree; // ScaleOffsetRange *
    unsigned int sor_idx_tree_size;

    unsigned long leaf_scale_offset;
    unsigned int max_path_len;

    size_t scales_count; // A scale corresponds to a leaf dimension member in a multidimensional model.

    AxisBuildAssist *assist;
} Axis;

Axis *ax_create(MemAllocMng *mam);

int ax_size(Axis *axis);

Axis *cs_get_axis(CoordinateSystem *cs, int axis_position);

void cs_add_axis(CoordinateSystem *cs, Axis *axis);

typedef struct _axis_scale
{
    __uint64_t *fragments;
    int fragments_len;
} Scale;

void Scale_print(Scale *);

void *scal__destory(void *scale);

int scal_cmp(void *_one, void *_other);

typedef struct _scale_offset_range
{
    md_gid gid; // ID of the non-measure dimension member (can be a leaf member or a non-leaf member)

    unsigned long start_position;
    unsigned long end_position;

    unsigned long offset;
} ScaleOffsetRange;

void ScaleOffsetRange_print(ScaleOffsetRange *);

int ScaleOffsetRange_cmp(void *obj, void *other);

void *ScaleOffsetRange_destory(void *);

int vce_append(EuclidCommand *ec);

void reload_space(unsigned long cs_id);

void do_solidify_mirror(unsigned long cs_id);

void vce_init();

void vce_load();

typedef struct _measure_space_
{
    __uint64_t id;
    size_t segment_count;
    size_t segment_scope;
    // RedBlackTree **tree_ls_h;
    char **data_ls_h;
    unsigned long *data_lens;
    int cell_vals_count;
} MeasureSpace;

void space_unload(__uint64_t id);

MeasureSpace *space_new(unsigned long id, size_t segment_count, size_t segment_scope, int cell_vals_count, MemAllocMng *mam);

void MeasureSpace_print(MeasureSpace *);

__uint64_t cs_axis_span(CoordinateSystem *cs, int axis_order);

void space_add_measure(MeasureSpace *space, __uint64_t measure_position, void *cell);

void space_plan(MeasureSpace *space, RedBlackTree **tmp_rbt_hs);

void space__destory(MeasureSpace *);

/*
 * @return ArrayList<GridData *>
 */
ArrayList *vce_vactors_values(MDContext *md_ctx, MddTuple **tuples_matrix_h, unsigned long v_len);

void dispatchAggregateMeasure(/*MDContext *md_context,*/ Cube *cube, ArrayList *direct_vectors, double **_measures_, char **_null_flags_, unsigned long *_len_);

void do_calculate_measure_value(MDContext *md_ctx, Cube *, MddTuple *, GridData *grid_data);


// Axis(struct _coordinate_axis) functions
Scale *ax_find_scale(Axis *axis, Scale *sample);


// Scale(struct _axis_scale) functions
void scal_init(Scale *scale);


/************************************************************************
 ************************************************************************/
ArrayList *worker_aggregate_measure(EuclidCommand *ec);

#endif