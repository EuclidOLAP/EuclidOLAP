#ifndef EUCLID__MATH__H
#define EUCLID__MATH__H 1


#define GRIDDATA_TYPE_NUM 0
#define GRIDDATA_TYPE_BOOL 1
#define GRIDDATA_TYPE_STR 2

#define GRIDDATA_BOOL_TRUE 1
#define GRIDDATA_BOOL_FALSE 0

typedef struct _grid_data_
{
    char null_flag;
    char type;
    char boolean;
    double val;
    char *str;
} GridData;

// typedef enum _cell_type_
// {
//     NIL, // nil is null
//     NUM,
//     BOOL,
//     STR,
//     NAN
// } CellType;

// typedef struct _vector_cell_
// {
//     CellType type;
//     union
//     {
//         double val;
//         int boolean;
//         char *str;
//     } option;

// } Cell;

#endif