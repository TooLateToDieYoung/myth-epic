#ifndef __MYTH_EPIC_HTTP_API
#define __MYTH_EPIC_HTTP_API

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Myth Epic Library */
#include "http.h"

/* declare handlers */
int templateHandler(http_s * const refs);

/* register api */
typedef struct {
    const char * const path;
    int (*handler)(http_s * const);
} api_s;

static api_s api[] = {
    { "/template", templateHandler },

/* end of api */
    {0}
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MYTH_EPIC_HTTP_API */
