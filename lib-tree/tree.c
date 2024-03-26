#include <assert.h>

#include "tree.h"

typedef struct node_s node_s;
struct node_s {
    void const * pvValue;
    node_s * psRefsP;
    node_s * psRefsL;
    node_s * psRefsR;
    size_t zHeight;
};

struct tree_s {
    pool_s * const psPool;
    int (* const pfCompare)(void *, void *);
    int (* const pfFree)(void *);

    node_s * psRoot;
    size_t zSize;

    node_s * psIter;
    volatile tree_iterator_state_e eIterState;
};

static node_s * _treeTidyUp(node_s * const psRefs);
static node_s * _treeFillUp(node_s * const psRefs);
static node_s * _treeSearch(node_s * const psRefs, void const * const psValue, int (* const pfCompare)(void *, void *), node_s ** ppsLast);
static size_t _treeHeight(node_s const * const psRefs);
static size_t _treeMax(const size_t zA, const size_t zB);
static void _treeLinkP(node_s * const psCenter, node_s * const psRefsP, node_s * const psOrigin);
static void _treeLinkL(node_s * const psCenter, node_s * const psRefsL);
static void _treeLinkR(node_s * const psCenter, node_s * const psRefsR);

/* public */
tree_s *
treeMake(
    pool_s * const psPool,
    int (* const pfCompare)(void *, void *),
    int (* const pfFree)(void *)
) {
assert(pfCompare);
assert(pfFree);

    tree_s * const psRefs = (tree_s *)poolAlloc(psPool, sizeof(tree_s));
    if ( NULL != psRefs )
    {
        *(void **)&psRefs->psPool = psPool;
        *(void **)&psRefs->pfCompare = pfCompare;
        *(void **)&psRefs->pfFree = pfFree;

        psRefs->psRoot = NULL;
        psRefs->zSize = 0;

        psRefs->psIter = NULL;
        psRefs->eIterState = TISReset;
    }

    return psRefs;
}

void 
treeFree(
    void * pvRefs
) {
    tree_s * const  psRefs = (tree_s *)( pvRefs );

    if ( NULL != psRefs )
    {
        while ( treeSize(psRefs) > 0 )
        {
            treeRemove(psRefs, psRefs->psRoot->pvValue);
        }

        poolErase(psRefs->psPool, psRefs);
    }
}

void *
treeAccess(
    tree_s * const psRefs, 
    void const * const pvValue
) {
    return ( NULL == psRefs ) ? ( NULL ) : _treeSearch(psRefs->psRoot, pvValue, psRefs->pfCompare, NULL) ;
}

tree_s *
treeInsert(
    tree_s * const psRefs, 
    void const * const pvValue
) {
    node_s * psCurr = NULL;
    node_s * psLast = NULL;

    if ( TISReset != treeIteratorState(psRefs) )
    {
        return psRefs;
    }

    psCurr = _treeSearch(psRefs->psRoot, pvValue, psRefs->pfCompare, &psLast);
    if ( NULL != psCurr )
    {
        return treeChange(psRefs, pvValue);
    }

    psCurr = (node_s *)poolAlloc(psRefs->psPool, sizeof(node_s));
    if ( NULL != psCurr )
    {
        psCurr->pvValue = pvValue;
        psCurr->zHeight = 1;
        psCurr->psRefsP = psLast;

        if ( NULL != psLast )
        {
            if ( 0 > psRefs->pfCompare(pvValue, psLast->pvValue) ) 
            { 
                psLast->psRefsL = psCurr; 
            } 
            else 
            { 
                psLast->psRefsR = psCurr; 
            }
        }

        // ? balance tree
        psRefs->psRoot = ( 0 == treeSize(psRefs) ) ? ( psCurr ) : _treeTidyUp(psLast) ;
        psRefs->zSize++;
    }
    else  // ! Error: calloc failed
    {
        return NULL;
    }

    return psRefs;
}

tree_s * 
treeChange(
    tree_s * const psRefs, 
    void const * const pvValue
) {
    node_s * psCurr = NULL;
    node_s * psLast = NULL;

    if ( TISReset != treeIteratorState(psRefs) )
    {
        return psRefs;
    }

    psCurr = _treeSearch(psRefs->psRoot, pvValue, psRefs->pfCompare, &psLast);
    if ( NULL == psCurr )
    {
        return treeInsert(psRefs, pvValue);
    }

    psRefs->pfFree(psCurr->pvValue);
    psCurr->pvValue = pvValue;

    return psRefs;
}

tree_s *
treeRemove(
    tree_s * const psRefs, 
    void const * const pvValue
) {
    node_s * psCurr = NULL;
    node_s * psDrop = NULL;

    if ( TISReset != treeIteratorState(psRefs) )
    {
        return psRefs;
    }

    psCurr = _treeSearch(psRefs->psRoot, pvValue, psRefs->pfCompare, NULL);
    if ( NULL != psCurr )
    {
        psRefs->pfFree(psCurr->pvValue);

        // ? fill up & remove the lastest one from this topology
        psDrop = _treeFillUp(psCurr);

        // ? tidy up this tree and reset the root
        psRefs->psRoot = ( 1 == treeSize(psRefs) ) ? ( NULL ) : _treeTidyUp(psDrop->psRefsP) ;
        psRefs->zSize--;

        // ? release useless node
        poolErase(psRefs->psPool, psDrop);
    }

    return psRefs;
}

size_t
treeSize(
    tree_s const * const psRefs
) {
    return ( NULL == psRefs ) ? ( 0 ) : ( psRefs->zSize ) ;
}

size_t
treeHeight(
    tree_s const * const psRefs
) {
    return ( NULL != psRefs ) ? ( 0 ) : _treeHeight(psRefs->psRoot) ;
}

void
treeIteratorBlock(
    tree_s * const psRefs
) {
    if ( NULL != psRefs )
    {
        psRefs->eIterState = TISBlock;
        psRefs->psIter = psRefs->psRoot;
    }
}

tree_iterator_s *
treeIteratorShift(
    tree_s * const psRefs
) {
    tree_iterator_s * psResult = NULL;

    if ( TISBlock != treeIteratorState(psRefs) )
    {
        return NULL;
    }

    if ( NULL == psRefs->psIter )
    {
        return NULL; /* last time already shift to the end */
    }

    psResult = (tree_iterator_s *)( psRefs->psIter );

    if ( NULL != psRefs->psIter->psRefsL )
    {
        psRefs->psIter = psRefs->psIter->psRefsL;
        goto __exit;
    }
    
    if ( NULL != psRefs->psIter->psRefsR )
    {
        psRefs->psIter = psRefs->psIter->psRefsR;
        goto __exit;
    }

    while ( NULL != psRefs->psIter->psRefsP )
    {
        if ( psRefs->psIter == psRefs->psIter->psRefsP->psRefsR )
        {
            psRefs->psIter = psRefs->psIter->psRefsP;
        }
        else
        {
            psRefs->psIter = psRefs->psIter->psRefsP->psRefsR;
            goto __exit;
        }
    }

    psRefs->psIter = NULL; /* this time attach to end of iterators */

__exit:
    return psResult; 
}

void
treeIteratorReset(
    tree_s * const psRefs
) {
    if ( NULL != psRefs )
    {
        psRefs->psIter = NULL;
        psRefs->eIterState = TISReset;
    }
}

tree_iterator_state_e
treeIteratorState(
    tree_s const * const psRefs
) {
    return ( NULL == psRefs ) ? ( TISError ) : ( psRefs->eIterState ) ;
}

/* private */
static 
node_s *
_treeTidyUp(
    node_s * const psRefs
) {
    node_s * psParent = NULL;
    node_s * psCenter = NULL;
    size_t zHL = 0;
    size_t zHR = 0;

    psParent = psRefs->psRefsP;
    zHL = _treeHeight(psRefs->psRefsL);
    zHR = _treeHeight(psRefs->psRefsR);

    if ( zHL > zHR + 1 )
    {
        psCenter = psRefs->psRefsL;

        _treeLinkL(psRefs, psCenter->psRefsR);
        _treeLinkR(psCenter, psRefs);
        _treeLinkP(psCenter, psParent, psRefs);

        zHL = _treeHeight(psRefs->psRefsL);
        psRefs->zHeight = 1 + _treeMax(zHL, zHR);
    }

    if ( zHL + 1 < zHR )
    {
        psCenter = psRefs->psRefsR;

        _treeLinkR(psRefs, psCenter->psRefsL);
        _treeLinkL(psCenter, psRefs);
        _treeLinkP(psCenter, psParent, psRefs);

        zHR = _treeHeight(psRefs->psRefsR);
        psRefs->zHeight = 1 + _treeMax(zHL, zHR);
    }

    zHL = _treeHeight(psCenter->psRefsL);
    zHR = _treeHeight(psCenter->psRefsR);
    psCenter->zHeight = 1 + _treeMax(zHL, zHR);

    return ( NULL == psParent ) ? ( psCenter ) : _treeTidyUp(psParent) ;
}

static 
node_s *
_treeFillUp(
    node_s * const psRefs
) {
    node_s * psWinner = NULL;

    // ? check if attach to the tail
    if ( 1 == _treeHeight(psRefs) )
    {
        // ? check if the tail has parent, which can refer to the tail: need to unlink
        if ( NULL != psRefs->psRefsP )
        {
            // ? make the tail be isolated
            if ( psRefs == psRefs->psRefsP->psRefsL )
            {
                psRefs->psRefsP->psRefsL = NULL;
            }
            else
            {
                psRefs->psRefsP->psRefsR = NULL;
            }
        }

        return psRefs; // ? release memory source by the caller
    }

    // ? store the next one data, and shift to the next level
    psWinner = psRefs->psRefsL ? psRefs->psRefsL : psRefs->psRefsR ;
    psRefs->pvValue = psWinner->pvValue;

    // ? keep looking for the tail
    return _treeFillUp(psWinner);
}

static 
node_s *
_treeSearch(
    node_s * const psRefs,
    void const * const pvValue,
    int (* const pfCompare)(void *, void *),
    node_s ** ppsLast
) {
    int check = 0;
    node_s * psTemp = NULL;
    node_s * psCurr = psRefs;

    while ( NULL != psCurr )
    {
        psTemp = psCurr;
        check = pfCompare(pvValue, psCurr->pvValue);
        if ( 0 > check ) { psCurr = psCurr->psRefsL; continue; }
        if ( 0 < check ) { psCurr = psCurr->psRefsR; continue; }
        break;
    }

    if ( NULL != ppsLast )
    {
        *ppsLast = psTemp;
    }
    return psCurr;
}

static
size_t
_treeHeight(
    node_s const * const psRefs
) {
    return ( NULL == psRefs ) ? ( 0 ) : ( psRefs->zHeight ) ;
}

static 
size_t 
_treeMax(
    const size_t zA, 
    const size_t zB
) {
    return ( zA > zB ) ? ( zA ) : ( zB ) ;
}

static 
void 
_treeLinkP(
    node_s * const psCenter, 
    node_s * const psRefsP, 
    node_s * const psOrigin
) {
    if ( NULL != psCenter )
    {
        psCenter->psRefsP = psRefsP;
    }
    if ( NULL != psRefsP )
    {
        if ( psOrigin == psRefsP->psRefsL ) { psRefsP->psRefsL = psCenter; return; }
        if ( psOrigin == psRefsP->psRefsR ) { psRefsP->psRefsR = psCenter; return; }
        // ! Error
    }
}

static 
void 
_treeLinkL(
    node_s * const psCenter, 
    node_s * const psRefsL
) {
    if ( NULL != psCenter )
    {
        psCenter->psRefsL = psRefsL;
    }
    if ( NULL != psRefsL )
    {
        psRefsL->psRefsP = psCenter;
    }
}

static 
void 
_treeLinkR(
    node_s * const psCenter, 
    node_s * const psRefsR
) {
    if ( NULL != psCenter )
    {
        psCenter->psRefsR = psRefsR;
    }
    if ( NULL != psRefsR )
    {
        psRefsR->psRefsP = psCenter;
    }
}
