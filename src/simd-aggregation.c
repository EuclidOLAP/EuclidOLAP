#include <immintrin.h> // use SIMD

#include "simd-aggregation.h"

static void do_sas_agg(SIMDAggSt *sas, int m256_grps)
{
    __m256d vector_1 = _mm256_load_pd(&(sas->elements[0]));

    for (int i = 1; i < m256_grps; i++)
    {
        __m256d vector_2 = _mm256_load_pd(&(sas->elements[i * 4]));
        vector_1 = _mm256_add_pd(vector_1, vector_2);
    }

    double *vals = (double *)&vector_1;
    for (int i = 0; i < 4; i++)
    {
        sas->summary_value += vals[i];
    }

    sas->null_flag = 0;
    sas->index = 0;
}

void sas_agg(SIMDAggSt *sas)
{
    if (sas->index == 0)
        return;

    sas->null_flag = 0;

    int _idx = 0;
    int m256_grps = sas->index / 4;

    int snapshoot = sas->index;

    if (m256_grps > 1)
    {
        do_sas_agg(sas, m256_grps);
        _idx = m256_grps * 4;
    }

    for (; _idx < snapshoot; _idx++)
    {
        sas->summary_value += sas->elements[_idx];
    }
}

void sas_agg_all(SIMDAggSt *sas)
{
    do_sas_agg(sas, 128);
}