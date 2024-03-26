#ifndef __MYTH_EPIC_LIB_TREE
#define __MYTH_EPIC_LIB_TREE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "pool.h"

typedef enum { TISError = -1, TISReset, TISBlock } tree_iterator_state_e;

typedef struct tree_s tree_s;

typedef struct {
    void const * const pvValue;
} tree_iterator_s;

tree_s * 
treeMake(
    pool_s * const psPool,
    int (* const pfCompare)(void *, void *),
    int (* const pfFree)(void *)
);

void 
treeFree(
    void * pvRefs
);

void * 
treeAccess(
    tree_s * const psRefs, 
    void const * const pvValue
);

tree_s * 
treeInsert(
    tree_s * const psRefs, 
    void const * const pvValue
);

tree_s * 
treeChange(
    tree_s * const psRefs, 
    void const * const pvValue
);

tree_s *
treeRemove(
    tree_s * const psRefs,
    void const * const pvValue
);

size_t 
treeSize(
    tree_s const * const psRefs
);

size_t 
treeHeight(
    tree_s const * const psRefs
);

void
treeIteratorBlock(
    tree_s * const psRefs
);

tree_iterator_s *
treeIteratorShift(
    tree_s * const psRefs
);

void
treeIteratorReset(
    tree_s * const psRefs
);

tree_iterator_state_e
treeIteratorState(
    tree_s const * const psRefs
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MYTH_EPIC_LIB_TREE */
