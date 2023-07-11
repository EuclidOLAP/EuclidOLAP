#include <string.h>

#include "mathematics.h"
#include "utils.h"

void cells_sum(GridData *celarr, int len, GridData **pp) {
    if (*pp == NULL)
        *pp = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);

    for (int i=0;i<len;i++) {
        (*pp)->val += celarr[i].val;
    }
}

void cells_max(GridData *celarr, int len, GridData **pp) {
    if (*pp == NULL)
        *pp = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);

    (*pp)->val = celarr[0].val;
    for (int i=1;i<len;i++) {
        if (celarr[i].val > (*pp)->val)
            (*pp)->val = celarr[i].val;
    }
}

void cells_min(GridData *celarr, int len, GridData **pp) {
    if (*pp == NULL)
        *pp = mam_alloc(sizeof(GridData), OBJ_TYPE__GridData, NULL, 0);

    (*pp)->val = celarr[0].val;
    for (int i=1;i<len;i++) {
        if (celarr[i].val < (*pp)->val)
            (*pp)->val = celarr[i].val;
    }

}

void cells_avg(GridData *celarr, unsigned int len, GridData *gdp) {
    gdp->type = GRIDDATA_TYPE_NUM;
    gdp->val = celarr[0].val;
    for (int i=1;i<len;i++) {
        gdp->val += celarr[i].val;
    }
    gdp->val = gdp->val / len;
}