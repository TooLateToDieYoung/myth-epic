#include "pool.h"
#include <assert.h>
#include <stdbool.h>

typedef struct {
    unsigned int headDiff : 14;
    unsigned int nextDiff : 14;
    unsigned int shiftDeg : 4;
} node_s;

typedef struct {
    node_s * psBoundary; 
    size_t zUsage;
} info_s;

struct pool_s {
    node_s psHead;
    info_s psInfo;
};

static 
unsigned int 
_poolComputeShiftDegree(
    const size_t zSize
) {
    unsigned int deg;
    size_t zDeg;

    if ( zSize > ( 1 << 14 ) )
    {
        // ! error: too large
        return 0;
    }

    /* check degree [3, 14] */
    for ( deg = 13; deg > 3; --deg )
    {
        zDeg = ( 1 << deg );
        if ( 0 != ( zDeg & zSize ) )
        {
            return ( zDeg == zSize ) ? ( deg ) : ( deg + 1 ) ;
        }
    }

    return deg; /* if less than 8 bytes, then assign 8 bytes = ( 1 << 3 ) */
}

pool_s *
poolFormat(
    void * const pvBuffer,
    const size_t zBufferSize
) {
    pool_s * psRefs = NULL;

    if ( NULL == pvBuffer || sizeof(pool_s) > zBufferSize )
    {
        return NULL;
    }

    psRefs = (pool_s *)( pvBuffer );

    psRefs->psHead.headDiff = 0;
    psRefs->psHead.nextDiff = 0;
    psRefs->psHead.shiftDeg = _poolComputeShiftDegree( sizeof(pool_s) );

    psRefs->psInfo.psBoundary = (node_s *)( (char *)pvBuffer + zBufferSize );
    psRefs->psInfo.zUsage = ( 1 << psRefs->psHead.shiftDeg );

    return psRefs;
}

void *
poolAlloc(
    pool_s * const psRefs,
    const size_t zAllocSize
) {
    node_s * psPrevNode = NULL;
    node_s * psCurrNode = NULL;
    node_s * psNextNode = NULL;
    node_s * psTempNode = NULL;
    node_s * psFakeNode = NULL;
    unsigned int shiftDeg = 0;

    if ( NULL == psRefs || 0 == zAllocSize || zAllocSize > ( 1 << 14 ) )
    {
        return NULL;
    }

    shiftDeg = _poolComputeShiftDegree( sizeof(node_s) + zAllocSize );
    if ( 0 == shiftDeg )
    {
        // ! error: out of range, access addr more then 16kB.
        return NULL;
    }
    
    psPrevNode = &psRefs->psHead;
    do {
        psCurrNode = (node_s *)( (char *)psPrevNode + psPrevNode->nextDiff );
        psNextNode = (node_s *)( (char *)psCurrNode + psCurrNode->nextDiff );
        psTempNode = (node_s *)( (char *)psCurrNode + ( 1 << psCurrNode->shiftDeg ) );
        psFakeNode = (node_s *)( (char *)psTempNode + ( 1 << shiftDeg ) );

        if ( 0 == psCurrNode->nextDiff )
        {
            if ( psRefs->psInfo.psBoundary >= psFakeNode )
            {
                // ? this gap is enough
                psTempNode->headDiff = (char *)psTempNode - (char *)&psRefs->psHead;
                psCurrNode->nextDiff = (char *)psTempNode - (char *)psCurrNode;
                psTempNode->nextDiff = 0;
                psTempNode->shiftDeg = shiftDeg;
                goto __exit;
            }
            else
            {
                // ! error: memory not avalible
                return NULL;
            }
        }
        else if ( psNextNode >= psFakeNode )
        {
            // ? this gap is enough
            psTempNode->headDiff = (char *)psTempNode - (char *)&psRefs->psHead;
            psCurrNode->nextDiff = (char *)psTempNode - (char *)psCurrNode;
            psTempNode->nextDiff = (char *)psNextNode - (char *)psTempNode;
            psTempNode->shiftDeg = shiftDeg;
            goto __exit;
        }

        psPrevNode = psCurrNode;
        psCurrNode = psNextNode;
    } while ( true );

__exit:
    psRefs->psInfo.zUsage += ( 1 << psTempNode->shiftDeg );
    return (void *)( psTempNode + 1 );
}

pool_s *
poolErase(
    pool_s * const psRefs,
    void * const pvTarget
) {
    node_s * psPrevNode = NULL;
    node_s * psCurrNode = NULL;
    node_s * psNextNode = NULL;

    if ( NULL == psRefs || NULL == pvTarget )
    {
        return psRefs;
    }

    if ( 0 == psRefs->psHead.nextDiff )
    {
        // ! error: there is no allocated memeory
        return NULL;
    }

    psPrevNode = &psRefs->psHead;
    psCurrNode = (node_s *)( (char *)psPrevNode + psPrevNode->nextDiff );
    if ( (void *)psCurrNode > pvTarget || (void *)psRefs->psInfo.psBoundary <= pvTarget )
    {
        // ! error: the pointer is not in the valid range
        return NULL;
    }

    while ( (void *)( psCurrNode + 1 ) != pvTarget )
    {
        if ( 0 == psCurrNode->nextDiff )
        {
            // ! error not found and no more node
            return NULL;
        }

        psPrevNode = psCurrNode;
        psCurrNode = (node_s *)( (char *)psPrevNode + psPrevNode->nextDiff );
    }

    if ( 0 == psCurrNode->nextDiff )
    {
        psPrevNode->nextDiff = 0;
    }
    else
    {
        psNextNode = (node_s *)( (char *)psCurrNode + psCurrNode->nextDiff );
        psNextNode->headDiff = (char *)psNextNode - (char *)&psRefs->psHead;
        psPrevNode->nextDiff = (char *)psNextNode - (char *)psPrevNode;
    }

    psRefs->psInfo.zUsage -= ( 1 << psCurrNode->shiftDeg );
    return psRefs;
}

size_t
poolSpace(
    pool_s const * const psRefs,
    void * const pvTarget
) {
    node_s * psCurrNode = NULL;

    if ( NULL == psRefs || NULL == pvTarget )
    {
        return psRefs;
    }

    if ( 0 == psRefs->psHead.nextDiff )
    {
        // ! error: there is no allocated memeory
        return 0;
    }

    psCurrNode = &psRefs->psHead;
    while ( 0 != psCurrNode->nextDiff )
    {
        if ( (void *)( psCurrNode + 1 ) == pvTarget )
        {
            // ? found it
            return ( 1 << psCurrNode->shiftDeg );
        }

        psCurrNode = (node_s *)( (char *)psCurrNode + psCurrNode->nextDiff );
    }

    return 0;
}

size_t
poolTotal(
    pool_s const * const psRefs
) {
    return ( NULL == psRefs ) ? ( 0 ) : ( (char *)psRefs->psInfo.psBoundary - (char *)psRefs ) ;
}

size_t
poolUsage(
    pool_s const * const psRefs
) {
    return ( NULL == psRefs ) ? ( 0 ) : ( psRefs->psInfo.zUsage ) ;
}
