#ifndef __MYTH_EPIC_LIB_WORD
#define __MYTH_EPIC_LIB_WORD

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "pool.h"

#warning "Mush call poolErase() to free"

typedef struct word_s word_s;

word_s * wordDupAll(pool_s * const psPool, char const * const pSource);
word_s * wordDupLen(pool_s * const psPool, char const * const pSource, const size_t zLength);
word_s * wordDupChr(pool_s * const psPool, char const * const pSource, const char endChr);
word_s * wordDupStr(pool_s * const psPool, char const * const pSource, char const * const pEndStr);

void wordFree(void * pvRefs);

char const * wordAccess(word_s const * const psRefs);
size_t wordLength(word_s const * const psRefs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MYTH_EPIC_LIB_WORD */
