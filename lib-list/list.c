#include <assert.h>
#include <stdbool.h>

#include "list.h"

typedef struct node_s node_s;
struct node_s
{
    void * pvValue;
    size_t zXor;
};

struct list_s
{
    pool_s * psPool;
    void (*pfFree)(void *);

    node_s * psHead;
    node_s * psTail;
    size_t zLength;

    node_s * psPrev;
    node_s * psCurr;
    node_s * psNext;
    size_t zRecord;
};

static bool _listTryAccess(list_s * const psRefs, const size_t zIndex);
static void _listQuickSort(node_s * const psHead, node_s * const psTail, int (*pfCompare)(void *, void *));

/* public */
list_s *
listMake(
    pool_s * const psPool,
    void (*pfFree)(void *)
) {
assert(pfFree);

    list_s * const psRefs = (list_s *)poolAlloc(psPool, sizeof(list_s));
    if ( NULL != psRefs )
    {
        psRefs->psPool = psPool;
        psRefs->pfFree = pfFree;
        psRefs->psHead = psRefs->psTail = psRefs->psPrev = psRefs->psCurr = psRefs->psNext = NULL;
        psRefs->zLength = psRefs->zRecord = 0;
    }

    return psRefs;
}

void 
listFree(
    void * pvRefs
) {
    list_s * psRefs = NULL;

    if ( NULL != pvRefs )
    {
        psRefs = (list_s *)pvRefs;

        while ( listLength(psRefs) > 0 )
        {
            listRemove(psRefs, 0);
        }

        poolErase(psRefs->psPool, pvRefs);
    }
}

void *
listAccess(
    list_s * const psRefs, 
    const size_t zIndex
) {
    return _listTryAccess(psRefs, zIndex) ? ( psRefs->psCurr->pvValue ) : ( NULL ) ;
}

list_s *
listInsert(
    list_s * const psRefs, 
    const size_t zIndex, 
    void * const pvValue
) {
    node_s * psTarget = NULL;

    if ( NULL != psRefs )
    {
        psTarget = (node_s *)poolAlloc(psRefs->psPool, sizeof(node_s));
        if ( NULL != psTarget )
        {
            psTarget->pvValue = pvValue;

            if ( 0 == listLength(psRefs) )
            {
                psRefs->psCurr = psRefs->psHead = psRefs->psTail = psTarget;
                psRefs->zRecord = 0;
            }
            else if ( zIndex == 0 ) // ? in front of the head
            {
                psTarget->zXor = (size_t)( psRefs->psHead );
                psRefs->psHead->zXor ^= (size_t)( psTarget );
                psRefs->psCurr = psRefs->psHead = psTarget;
                psRefs->zRecord = 0;
            }
            else if ( zIndex >= listLength(psRefs) ) // ? append to the tail
            {
                psTarget->zXor = (size_t)( psRefs->psTail );
                psRefs->psTail->zXor ^= (size_t)( psTarget );
                psRefs->psCurr = psRefs->psTail = psTarget;
                psRefs->zRecord = listLength(psRefs);
            }
            else if ( true == _listTryAccess(psRefs, zIndex) )
            {
                psTarget->zXor = ( (size_t)( psRefs->psPrev ) ^ (size_t)( psRefs->psCurr ) );

                psRefs->psPrev->zXor ^= (size_t)( psRefs->psCurr );
                psRefs->psPrev->zXor ^= (size_t)( psTarget );

                psRefs->psCurr->zXor ^= (size_t)( psRefs->psPrev );
                psRefs->psCurr->zXor ^= (size_t)( psTarget );

                psRefs->psCurr = psTarget;
                psRefs->zRecord = zIndex;
            }
            else // ! Error: cannot find correct position
            {
                poolErase(psRefs->psPool, psTarget);
                return NULL;
            }

            psRefs->zLength++;
        }
        else  // ! Error: calloc failed
        {
            return NULL;
        }
    }

    return psRefs;
}

list_s *
listChange(
    list_s * const psRefs, 
    const size_t zIndex, 
    void * const pvValue
) {
    list_s * psRet = psRefs;

    if ( true != _listTryAccess(psRefs, zIndex) )
    {
        return listInsert(psRefs, zIndex, pvValue);
    }
    else
    {
        psRefs->pfFree(psRefs->psCurr->pvValue);
        psRefs->psCurr->pvValue = pvValue;
    }

    return psRefs;
}

list_s * 
listRemove(
    list_s * const psRefs, 
    const size_t zIndex
) {
    node_s * psTarget = NULL;

    if ( true == _listTryAccess(psRefs, zIndex) )
    {
        psTarget = psRefs->psCurr;

        if ( 1 == listLength(psRefs) )
        {
            psRefs->psCurr = psRefs->psHead = psRefs->psTail = NULL;
            psRefs->zRecord = 0;
        }
        else if ( zIndex == 0 )
        {
            psRefs->psNext->zXor ^= (size_t)( psRefs->psCurr );
            psRefs->psHead = psRefs->psCurr = psRefs->psNext;
            psRefs->psNext = (node_s *)( psRefs->psHead->zXor );
            psRefs->zRecord = 0;
        }
        else if ( zIndex == listLength(psRefs) - 1 )
        {
            psRefs->psPrev->zXor ^= (size_t)( psRefs->psCurr );
            psRefs->psTail = psRefs->psCurr = psRefs->psPrev;
            psRefs->psPrev = (node_s *)( psRefs->psTail->zXor );
            psRefs->zRecord = zIndex - 1;
        }
        else
        {
            psRefs->psPrev->zXor ^= (size_t)( psRefs->psCurr );
            psRefs->psPrev->zXor ^= (size_t)( psRefs->psNext );

            psRefs->psNext->zXor ^= (size_t)( psRefs->psCurr );
            psRefs->psNext->zXor ^= (size_t)( psRefs->psPrev );

            psRefs->psCurr = psRefs->psNext;
            psRefs->psNext = (node_s *)( psRefs->psCurr->zXor ^ (size_t)( psRefs->psPrev ) );

            psRefs->zRecord = zIndex;
        }

        psRefs->pfFree(psTarget->pvValue);
        poolErase(psRefs->psPool, psTarget);

        psRefs->zLength--;
    }

    return psRefs;
}

list_s *
listRevert(
    list_s * const psRefs
) {
    if ( listLength(psRefs) > 1 )
    {
        psRefs->psCurr = psRefs->psTail;
        psRefs->psTail = psRefs->psHead;
        psRefs->psHead = psRefs->psCurr;

        psRefs->psPrev = NULL;
        psRefs->psNext = (node_s *)( psRefs->psHead->zXor );

        psRefs->zRecord = 0;
    }

    return psRefs;
}

size_t
listLength(
    list_s const * const psRefs
) {
    return ( NULL != psRefs ) ? ( psRefs->zLength ) : ( 0 ) ;
}

list_s *
listQuickSort(
    list_s * const psRefs,
    int (*pfCompare)(void *, void *)
) {
assert( NULL != pfCompare );

    if ( listLength(psRefs) > 1 )
    {
        _listQuickSort(psRefs->psHead, psRefs->psTail, pfCompare);
    }

    return psRefs;
}

/* private */
static 
bool 
_listTryAccess(
    list_s * const psRefs, 
    const size_t zIndex
) {
    node_s * psTemp = NULL;

    if ( NULL == psRefs )
    {
        return false;
    }

    if ( zIndex >= listLength(psRefs) )
    {
        return false;
    }

    if ( zIndex == 0 )
    {
        psRefs->psCurr = psRefs->psHead;
        psRefs->psPrev = NULL;
        psRefs->psNext = (node_s *)( psRefs->psCurr->zXor );
        psRefs->zRecord = zIndex;
    }
    else if ( zIndex == listLength(psRefs) - 1 )
    {
        psRefs->psCurr = psRefs->psTail;
        psRefs->psPrev = (node_s *)( psRefs->psCurr->zXor );
        psRefs->psNext = NULL;
        psRefs->zRecord = zIndex;
    }
    else
    {
        while ( psRefs->zRecord < zIndex )
        {
            psTemp = psRefs->psCurr;
            psRefs->psPrev = psRefs->psCurr;
            psRefs->psCurr = psRefs->psNext;
            psRefs->psNext = (node_s *)( psRefs->psNext->zXor ^ (size_t)( psTemp ) );
            psRefs->zRecord++;
        }
        while ( psRefs->zRecord > zIndex )
        {
            psTemp = psRefs->psCurr;
            psRefs->psNext = psRefs->psCurr;
            psRefs->psCurr = psRefs->psPrev;
            psRefs->psPrev = (node_s *)( psRefs->psPrev->zXor ^ (size_t)( psTemp ) );
            psRefs->zRecord--;
        }
    }

    return true;
}

static 
void 
_listQuickSort(
    node_s * const psHead, 
    node_s * const psTail, 
    int (*pfCompare)(void *, void *)
) {
    // TODO
}
