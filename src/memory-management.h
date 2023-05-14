#ifndef EUCLID__MEM_MANG_H
#define EUCLID__MEM_MANG_H 1

#include <stdlib.h>

#define allocate_memory(__size) malloc(__size)

#define release_memory(__ptr) free(__ptr)

#endif