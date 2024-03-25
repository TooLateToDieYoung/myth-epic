#ifndef __MYTH_EPIC_LIB_LIST
#define __MYTH_EPIC_LIB_LIST

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "pool.h"

typedef struct list_s list_s;

list_s * 
listMake(
    pool_s * const psPool,
    void (*pfFree)(void *)
);

void 
listFree(
    void * pvRefs
);

void * 
listAccess(
    list_s * const psRefs, 
    const size_t zIndex
);

list_s * 
listInsert(
    list_s * const psRefs, 
    const size_t zIndex, 
    void * const pvValue
);

list_s * 
listChange(
    list_s * const psRefs, 
    const size_t zIndex, 
    void * const pvValue
);

list_s *
listRemove(
    list_s * const psRefs, 
    const size_t zIndex
);

list_s *
listRevert(
    list_s * const psRefs
);

size_t 
listLength(
    list_s const * const psRefs
);

list_s * 
listQuickSort(
    list_s * const psRefs,
    int (*pfCompare)(void *, void *)
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MYTH_EPIC_LIB_LIST */
