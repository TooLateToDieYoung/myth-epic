#ifndef __MYTH_EPIC_LIB_WORD
#define __MYTH_EPIC_LIB_WORD

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "pool.h"
#include <string.h>

static inline 
char * 
wordStrdup(
    pool_s * const psPool,
    char const * const pSource,
    const size_t zLength
) {
    size_t zMinmum = 0;
    char * pResult = 0;
    
    if ( NULL == psPool || NULL == pSource )
    {
        return NULL;
    }

    zMinmum = 1 + strlen(pSource);
    if ( zLength > 0 && zMinmum > zLength )
    {
        zMinmum = zLength;
    }

    if ( 0 == zMinmum )
    {
        return NULL;
    }

    pResult = (char *)poolAlloc(psPool, zMinmum);
    if ( NULL != pResult )
    {
        memcpy(pResult, pSource, zMinmum - 1);
    }

    return pResult;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MYTH_EPIC_LIB_WORD */
