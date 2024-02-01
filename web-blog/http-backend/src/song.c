/* C89 Std. */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Myth Epic Library */
#include "http.h"
#include "api.h"
#include "json.h"

#define DATA_SONG_MENU_PATH "data/song/menu.json"

/* private */
static int _get(http_s * const refs);

/* public */
int songHandler(http_s * const refs)
{
    fprintf(stdout, "[INFO] enter: %s\n", __FUNCTION__);

    if ( NULL != refs )
    {
        switch ( refs->req.method )
        {
            case HMGet: { return _get(refs); }
            default: { break; }
        }
    }

    return -1;
}

/* private */
static int _get(http_s * const refs)
{
    json_s * menu = NULL;
    json_s * result = NULL;
    const char * queryParam = NULL;
    size_t songId = 0;
    json_s * songInfo = NULL;
    const char * songPath = NULL;
    size_t songLen = 0;

    strncat(
        refs->res.header, 
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Type: application/json\r\n"
        "Content-Security-Policy: default-src 'self'\r\n", 
        sizeof(refs->res.header)
    );

    refs->res.status = HSNotFound; // ? set default response status

    menu = jsonParseFromFile(DATA_SONG_MENU_PATH);
    if ( NULL == menu ) 
    {
        snprintf(refs->msg, sizeof(refs->msg), "cannot parse menu: %s", DATA_SONG_MENU_PATH) ;
        goto __exit; 
    }

    queryParam = strstr(refs->query, "id=");
    if ( NULL != queryParam ) 
    {
        songId = (size_t)strtoull(queryParam + strlen("id="), NULL, 10);
        if ( songId >= jsonArrLength(menu) ) 
        { 
            snprintf(refs->msg, sizeof(refs->msg), "song id = %zu out of range = %zu", songId, jsonArrLength(menu)) ;
            goto __exit; 
        }

        songInfo = jsonArrAccess(menu, songId);
        if ( NULL == songInfo ) 
        { 
            snprintf(refs->msg, sizeof(refs->msg), "cannot find song id = %zu", songId) ;
            goto __exit; 
        }

        songPath = jsonGetStr(jsonObjAccess(songInfo, "path"));
        if ( NULL == songPath ) 
        { 
            snprintf(refs->msg, sizeof(refs->msg), "internal error") ;
            goto __exit; 
        }

        result = jsonParseFromFile(songPath);
        if ( NULL == result ) 
        { 
            snprintf(refs->msg, sizeof(refs->msg), "cannot parse json: %s", songPath) ;
            goto __exit; 
        }
    }
    else
    {
        result = menu;
        menu = NULL;
    }

    songLen = 1 + jsonStringify(result, NULL, 0);
    refs->res.body = (char *)calloc(songLen, sizeof(char));
    if ( NULL == refs->res.body )
    { 
        snprintf(refs->msg, sizeof(refs->msg), "system error: %s", strerror(errno)) ;
        goto __exit; 
    }

    jsonStringify(result, refs->res.body, songLen);
    refs->res.status = HSOk;

__exit:
    if ( NULL != menu ) { jsonFree(menu); }
    if ( NULL != result ) { jsonFree(result); }

    return HSOk != refs->res.status;
}
