#ifndef EUCLID__VCE_H
#define EUCLID__VCE_H 1

#include "command.h"
#include "utils.h"
#include "rb-tree.h"
#include "mdd.h"
#include "mdx.h"

#define SPACE_DEF_PARTITION_COUNT 128
#define SPACE_DEF_PARTITION_SPAN_MIN 270000

typedef struct _coordinate_system
{
    __uint64_t id;
    ArrayList *axes;
} CoordinateSystem;

CoordinateSystem *coosys_new(unsigned long id, int axes_count, MemAllocMng *mam);

void CoordinateSystem__gen_auxiliary_index(CoordinateSystem *);
void CoordinateSystem__calculate_offset(CoordinateSystem *);

typedef struct _coordinate_axis
{
    RedBlackTree *rbtree;
    RedBlackTree *sor_idx_tree; // ScaleOffsetRange *
    char *index;
    unsigned long coor_offset;
    unsigned int max_path_len;
} Axis;

Axis *ax_create(MemAllocMng *mam);

void ax_reordering(Axis *axis);
int ax_size(Axis *axis);

Axis *cs_get_axis(CoordinateSystem *cs, int axis_position);

void cs_add_axis(CoordinateSystem *cs, Axis *axis);

typedef struct _axis_scale
{
    __uint64_t *fragments;
    int fragments_len;
} Scale;

void Scale_print(Scale *);

Scale *scal_create();

void *scal__destory(void *scale);

int scal_cmp(void *_one, void *_other);

// void scal_set_len(Scale *scale, int fgs_len);

void scal_put_fragments(Scale *scale, int fgs_len, void *fragments);

Scale *scal__alloc(int fgs_len, void *fragments);

typedef struct _scale_offset_range
{
    md_gid gid; // The ID of the detail or fee detail dimension member

    unsigned long start_position;
    unsigned long end_position;

    unsigned long offset;

    unsigned long start_offset;
    unsigned long end_offset;
} ScaleOffsetRange;

ScaleOffsetRange *ScaleOffsetRange_create();

void ScaleOffsetRange_print(ScaleOffsetRange *);

int ScaleOffsetRange_cmp(void *obj, void *other);

void *ScaleOffsetRange_destory(void *);

void ax_set_scale(Axis *axis, Scale *scale);

int vce_append(EuclidCommand *ec);

void reload_space(unsigned long cs_id);

void vce_init();

void vce_load();

typedef struct _measure_space_
{
    __uint64_t id;
    size_t segment_count;
    size_t segment_scope;
    RedBlackTree **tree_ls_h;
    void **data_ls_h;
    unsigned long *data_lens;
    int cell_vals_count;
} MeasureSpace;

void space_unload(__uint64_t id);

// TODO about to be deprecated, replaced by the function space_new.
MeasureSpace *space_create(size_t segment_count, size_t segment_scope, int cell_vals_count);

MeasureSpace *space_new(unsigned long id, size_t segment_count, size_t segment_scope, int cell_vals_count, MemAllocMng *mam);

void MeasureSpace_print(MeasureSpace *);

__uint64_t ax_scale_position(Axis *axis, int fgs_len, void *fragments);

__uint64_t cs_axis_span(CoordinateSystem *cs, int axis_order);

void space_add_measure(MeasureSpace *space, __uint64_t measure_position, void *cell);

void space_plan(MeasureSpace *space);

void space__destory(MeasureSpace *);

double *vce_vactors_values(MDContext *md_ctx, MddTuple **tuples_matrix_h, unsigned long v_len, char **null_flags);

void do_calculate_measure_value(MDContext *md_ctx, Cube *, MddTuple *, GridData *grid_data);

#endif