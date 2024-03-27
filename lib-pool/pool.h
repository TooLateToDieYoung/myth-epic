#ifndef __MYTH_EPIC_LIB_POOL
#define __MYTH_EPIC_LIB_POOL

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

typedef struct pool_s pool_s;

pool_s *
poolFormat(
    void * const pvBuffer,
    const size_t zBufferSize
);

void *
poolAlloc(
    pool_s * const psRefs,
    const size_t zAllocSize
);

pool_s *
poolErase(
    pool_s * const psRefs,
    void * const pvTarget
);

size_t
poolSpace(
    pool_s const * const psRefs,
    void * const pvTarget
);

size_t
poolTotal(
    pool_s const * const psRefs
);

size_t
poolUsage(
    pool_s const * const psRefs
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MYTH_EPIC_LIB_TREE */
