/* C89 Std. */
#include <stdlib.h>
#include <string.h>

/* Myth Epic Library */
#include "http.h"
#include "api.h"
#include "json.h"

/* private */
static int _get(http_s * const refs);
static int _patch(http_s * const refs);
static int _put(http_s * const refs);
static int _post(http_s * const refs);
static int _delete(http_s * const refs);
static int _option(http_s * const refs);
static int _head(http_s * const refs);
static int _connect(http_s * const refs);
static int _trace(http_s * const refs);

/* public */
int templateHandler(http_s * const refs)
{
    fprintf(stdout, "[INFO] enter: %s\n", __FUNCTION__);

    if ( NULL != refs )
    {
        fprintf(stdout, "[DEBUG] refs->req.header =\n%s\n", refs->req.header);

        switch ( refs->req.method )
        {
            case HMGet: { return _get(refs); }
            case HMPatch: { return _patch(refs); }
            case HMPut: { return _put(refs); }
            case HMPost: { return _post(refs); }
            case HMDelete: { return _delete(refs); }
            case HMOption: { return _option(refs); }
            case HMHead: { return _head(refs); }
            case HMConnect: { return _connect(refs); }
            case HMTrace: { return _trace(refs); }
            default: { break; }
        }
    }

    return -1;
}

/* private */
static int _get(http_s * const refs)
{
    strncat(
        refs->res.header, 
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Type: application/json\r\n"
        "Content-Security-Policy: default-src 'self'\r\n", 
        sizeof(refs->res.header)
    );

    json_s * const jobj = jsonMakeObj();
    if ( NULL != jobj )
    {
        if ( jobj == jsonObjInsert(jobj, "template", jsonMakeBoo(true)) )
        {
            jsonDump(jobj, stdout);
        }
        else
        {
            snprintf(refs->msg, sizeof(refs->msg), "[Error] json insert obj error\n");
        }

        const size_t len = 1 + jsonStringify(jobj, NULL, 0);
        refs->res.body = (char *)calloc(len, sizeof(char));
        if ( NULL != refs->res.body )
        {
            jsonStringify(jobj, refs->res.body, len);
            refs->res.status = HSOk;
        }

        jsonFree(jobj);
    }
    else
    {
        fprintf(stderr, "[ERROR] cannot make json obj\n");
    }

    return 0;
}
static int _patch(http_s * const refs)
{
    // TODO
    return 0;
}
static int _put(http_s * const refs)
{
    // TODO
    return 0;
}
static int _post(http_s * const refs)
{
    // TODO
    return 0;
}
static int _delete(http_s * const refs)
{
    // TODO
    return 0;
}
static int _option(http_s * const refs)
{
    // TODO
    return 0;
}
static int _head(http_s * const refs)
{
    // TODO
    return 0;
}
static int _connect(http_s * const refs)
{
    // TODO
    return 0;
}
static int _trace(http_s * const refs)
{
    // TODO
    return 0;
}