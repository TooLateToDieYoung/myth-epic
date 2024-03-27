#include "word.h"

#include <string.h>

struct word_s {
    pool_s * const psPool;
    char * pString;
    size_t zLength;
};

word_s * 
wordDupAll(
    pool_s * const psPool,
    char const * const pSource
) {
    return wordDupChr(psPool, pSource, '\0');
}

word_s * 
wordDupLen(
    pool_s * const psPool,
    char const * const pSource,
    const size_t zLength
) {
    word_s * psRefs = 0;
    
    if ( NULL == psPool || NULL == pSource )
    {
        return NULL;
    }

    psRefs = (word_s *)poolAlloc(psPool, sizeof(word_s));
    if ( NULL == psRefs )
    {
        return NULL;
    }

    psRefs->zLength = 1 + strlen(pSource);
    if ( zLength > 0 && psRefs->zLength > zLength )
    {
        psRefs->zLength = zLength;
    }

    psRefs->pString = (char *)poolAlloc(psPool, psRefs->zLength);
    if ( NULL == psRefs->pString )
    {
        poolErase(psPool, psRefs);
        return NULL;
    }

    memcpy(psRefs->pString, pSource, psRefs->zLength - 1);
    psRefs->pString[ psRefs->zLength - 1 ] = '\0';

    *(void **)&psRefs->psPool = psPool;

    return psRefs;
}

word_s * 
wordDupChr(
    pool_s * const psPool,
    char const * const pSource,
    const char endChr
) {
    char const * pLocation = NULL;
    
    if ( NULL == psPool || NULL == pSource )
    {
        return NULL;
    }

    pLocation = strchr(pSource, endChr);
    return ( NULL == pLocation ) ? ( NULL ) : wordDupLen(psPool, pSource, 1 + pLocation - pSource) ;
}

word_s * 
wordDupStr(
    pool_s * const psPool,
    char const * const pSource,
    char const * const pEndStr
) {
    char const * pLocation = NULL;
    
    if ( NULL == psPool || NULL == pSource )
    {
        return NULL;
    }

    pLocation = strstr(pSource, pEndStr);
    return ( NULL == pLocation ) ? ( NULL ) : wordDupLen(psPool, pSource, 1 + pLocation - pSource) ;
}

void 
wordFree(
    void * pvRefs
) {
    word_s * psRefs = (word_s *)( pvRefs );

    if ( NULL != psRefs )
    {
        poolErase(psRefs->psPool, psRefs->pString);
        poolErase(psRefs->psPool, psRefs);
    }
}

char const * 
wordAccess(
    word_s const * const psRefs
) {
    return ( NULL == psRefs ) ? ( NULL ) : ( psRefs->pString ) ;
}

size_t 
wordLength(
    word_s const * const psRefs
) {
    return ( NULL == psRefs ) ? ( NULL ) : ( psRefs->zLength ) ;
}
