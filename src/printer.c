#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "printer.h"
#include "utils.h"

void _print_mdrs_table(EuclidCommand *ddrs)
{
    char *idx = ddrs->bytes + sizeof(int) + sizeof(short);
    int axes_count = *(int *)idx;
    idx += sizeof(int);

    assert(axes_count == 2);
    MemAllocMng *mam = MemAllocMng_new();

    char **rc_ps[2] = {NULL, NULL};

    int col_w, col_h, row_w, row_h, tb_w, tb_h, val_w, val_h;

    for (int i = 0; i < axes_count; i++)
    {
        int s_len = *(int *)idx;
        idx += sizeof(int);
        int t_len = *(int *)idx;
        idx += sizeof(int);

        if (i == 0)
        {
            row_w = t_len;
            row_h = s_len;
        }
        else
        {
            col_w = s_len;
            col_h = t_len;
        }

        rc_ps[i] = mam_alloc(s_len * t_len * sizeof(char *), OBJ_TYPE__RAW_BYTES, mam, 0);

        for (int s = 0; s < s_len; s++)
        {
            for (int t = 0; t < t_len; t++)
            {
                unsigned long member_id = *(unsigned long *)idx;
                idx += sizeof(unsigned long);
                char *member_name = idx;
                idx += strlen(member_name) + 1;

                if (i == 1)
                    rc_ps[i][t * s_len + s] = member_name;
                else
                    rc_ps[i][s * t_len + t] = member_name;
            }
        }
    }

    val_w = col_w;
    val_h = row_h;
    tb_w = row_w + col_w;
    tb_h = row_h + col_h;

    unsigned long rs_len = *(unsigned long *)idx;
    idx += sizeof(unsigned long);

    double *values = (double *)idx;
    idx += sizeof(double) * rs_len;

    char *null_flag_arr = idx;
    idx += sizeof(char) * rs_len;

    char **gdstrs = mam_alloc(rs_len * sizeof(char *), OBJ_TYPE__RAW_BYTES, mam, 0);
    for (int i=0;i<rs_len;i++) {
        gdstrs[i] = idx;
        // printf("@>>>>>> [%s]\n", idx);
        idx += strlen(idx) + 1;
    }

    char **measure_values_str = mam_alloc(rs_len * sizeof(char *), OBJ_TYPE__RAW_BYTES, mam, 0);

    for (int i = 0; i < rs_len; i++)
    {
        if (null_flag_arr[i])
        {
            measure_values_str[i] = "null";
        }
        else
        {
            measure_values_str[i] = mam_alloc(64, OBJ_TYPE__RAW_BYTES, mam, 0);
            sprintf(measure_values_str[i], "%.2lf", values[i]);
        }
    }

    char **table = mam_alloc(tb_w * tb_h * sizeof(char *), OBJ_TYPE__RAW_BYTES, mam, 0);

    for (int i = 0; i < tb_h; i++)
    {
        for (int j = 0; j < tb_w; j++)
        {
            if (i < col_h && j < row_w)
            {
                table[i * tb_w + j] = "";
                continue;
            }

            if (i < col_h && j >= row_w)
            {
                table[i * tb_w + j] = rc_ps[1][i * col_w + j - row_w];
                continue;
            }

            if (i >= col_h && j < row_w)
            {
                table[i * tb_w + j] = rc_ps[0][(i - col_h) * row_w + j];
                continue;
            }

            if (*gdstrs[(i - col_h) * col_w + j - row_w]) {
                table[i * tb_w + j] = gdstrs[(i - col_h) * col_w + j - row_w];
            } else {
                table[i * tb_w + j] = measure_values_str[(i - col_h) * col_w + j - row_w];
            }
            
        }
    }

    int *max_len_arr = mam_alloc(tb_w * sizeof(int), OBJ_TYPE__RAW_BYTES, mam, 0);

    for (int i = 0; i < tb_h; i++)
    {
        for (int j = 0; j < tb_w; j++)
        {
            char *str = table[i * tb_w + j];
            if (strlen(str) > max_len_arr[j])
                max_len_arr[j] = strlen(str);
        }
    }

    for (int i = 0; i < tb_h; i++)
    {
        for (int j = 0; j < tb_w; j++) {
            printf("+");
            for (int x = 0; x < max_len_arr[j] + 2; x++)
            {
                printf("-");
            }
        }
        printf("+\n");

        for (int j = 0; j < tb_w; j++)
        {
            printf("| ");
            char *str = table[i * tb_w + j];
            printf("%s", str);
            for (int x = 0; x < max_len_arr[j] - strlen(str); x++)
            {
                printf(" ");
            }
            printf(" ");
        }
        printf("|\n");
    }

    for (int j = 0; j < tb_w; j++) {
        printf("+");
        for (int x = 0; x < max_len_arr[j] + 2; x++)
        {
            printf("-");
        }
    }
    printf("+\n");

    mam_reset(mam);
    obj_release(mam->current_block);
    obj_release(mam);
}

void print_mdrs(EuclidCommand *mdrs)
{
    printf("\n\n");
    char *idx = mdrs->bytes + sizeof(int);
    assert(*(unsigned short *)idx == INTENT__MULTIDIM_RESULT_BIN);
    idx += sizeof(short);

    int axes_count = *(int *)idx;
    idx += sizeof(int);

    if (axes_count == 2)
    {
        _print_mdrs_table(mdrs);
        return;
    }

    for (int i = 0; i < axes_count; i++)
    {
        printf(">>> Axis(%d) >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", i);
        int s_len = *(int *)idx;
        idx += sizeof(int);
        int t_len = *(int *)idx;
        idx += sizeof(int);
        for (int s = 0; s < s_len; s++)
        {
            for (int t = 0; t < t_len; t++)
            {
                unsigned long member_id = *(unsigned long *)idx;
                idx += sizeof(unsigned long);
                char *member_name = idx;
                idx += strlen(member_name) + 1;

                printf("%lu %s\n", member_id, member_name);
            }
        }
    }

    printf("######################################################\n");
    printf("#                   Measure Values                   #\n");
    printf("######################################################\n");
    unsigned long rs_len = *(unsigned long *)idx;
    idx += sizeof(unsigned long);
    char *null_flag_arr = idx + sizeof(double) * rs_len;
    double *values = (double *)idx;
    for (int i = 0; i < rs_len; i++)
    {
        if (null_flag_arr[i])
        {
            printf("null\n");
        }
        else
        {
            printf("%lf\n", values[i]);
        }
    }
}