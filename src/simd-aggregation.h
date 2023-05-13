#ifndef EUCLID__SIMD_AGG_H
#define EUCLID__SIMD_AGG_H 1

// #include <immintrin.h> // use SIMD

#define SAS_ELS_LEN (128 * 4)

typedef struct _simd_agg_st_
{
    double elements[SAS_ELS_LEN];

    double summary_value;
    int null_flag; // 0 - not null, 1 - is null

    int index;
} SIMDAggSt;

void sas_agg(SIMDAggSt *sas);

void sas_agg_all(SIMDAggSt *sas);

#endif