#ifndef __MYTH_EPIC_HTTP
#define __MYTH_EPIC_HTTP

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum { 
    HMGet, /* get data */
    HMPatch, /* change partial data */
    HMPut, /* overwrite data */
    HMPost, /* create new data */
    HMDelete, /* delete data */
    HMOption, /* communication option */
    HMHead, /* same as method 'GET', but without response body */
    HMConnect, /* do handshake to make the tunnel */
    HMTrace, /* do loop-back test */
    HMUnknown
} http_method_e;

typedef enum {
/* Informational: 1xx */
    HSContinue = 100,
    HSSwitchingProtocols = 101,
    HSProcessing = 102,
    HSEarlyHints = 103,

/* Successful: 2xx */
    HSOk = 200, /* for http method 'GET', 'HEAD', 'PUT', 'POST', and 'TRACE' */
    HSCreated = 201, /* for 'POST' and some 'PUT' */
    HSAccepted = 202,
    HSNonAuthoritativeInformation = 203, /* the 200 OK response is preferred to this status */
    HSNoContent = 204,
    HSResetContent = 205,
    HSPartialContent = 206,
    HSMultiStatus = 207,
    HSAlreadyReported = 208,
    HSIMUsed = 209,

/* Redirection: 3xx */
    HSMultipleChoices = 300,
    HSMovedPermanently = 301,
    HSFound = 302,
    HSSeeOther = 303,
    HSNotModified = 304,
    HSUseProxy = 305,
//? reserved = 306, => reserved for old method no supported in HTTP/1.1
    HSTemporaryRedirect = 307,
    HSPermanentRedirect = 308,

/* Client Error: 4xx */
    HSBadRequest = 400,
    HSUnauthorized = 401,
    HSPaymentRequired = 402,
    HSForbidden = 403,
    HSNotFound = 404,
    HSMethodNotAllowed = 405,
    HSNotAcceptable = 406,
    HSProxyAuthenticationRequired = 407,
    HSRequestTimeout = 408,
    HSConflict = 409,
    HSGone = 410,
    HSLengthRequired = 411, /* request header 'Content-Length' is required */
    HSPreconditionFailed = 412,
    HSPayloadTooLarge = 413, /* request entity is larger than limits defined by server */
    HSUriTooLong = 414, /* the URI requested by the client is longer than the server is willing to interpret */
    HSUnsupportedMediaType = 415,
    HSRangeNotSatisfiable = 416, /* the range specified by the Range header field in the request cannot be fulfilled */
    HSExpectationFailed = 417, /* the Expect request header field cannot be met by the server */
    HSImATeapot = 418, /* the server refuses the attempt to brew coffee with a teapot */
    HSMisdirectedRequest = 421, 
    HSUnprocessableContent = 422, 
    HSLocked = 423, 
    HSFailedDependency = 424, 
    HSTooEarly = 425, /* indicates that the server is unwilling to risk processing a request that might be replayed */
    HSUpgradeRequired = 426, 
    HSPreconditionRequired = 428, 
    HSTooManyRequests = 429, /* the user has sent too many requests in a given amount of time ("rate limiting") */
    HSRequestHeaderFieldsTooLarge = 431, /* the server is unwilling to process the request because its header fields are too large */
    HSUnavailableForLegalReasons = 451,

/* Server Error: 5xx */
    HSInternalServerError = 500,
    HSNotImplemented = 501, /* the only methods that servers are required to support are 'GET' and 'HEAD' */
    HSBadGateway = 502,
    HSServiceUnavailable = 503,
    HSGatewayTimeout = 504, /* this error response is given when the server is acting as a gateway and cannot get a response in time */
    HSHttpVersionNotSupported = 505,
    HSVariantAlsoNegotiates = 506,
    HSInsufficientStorage = 507,
    HSLoopDetected = 508,
    HSNotExtended = 510,
    HSNetworkAuthenticationRequired = 511,

/* the of all response status code */
    HSUnknown
} http_status_e;

typedef enum {
/* text/? */
    HCTTextHtml,
    HCTTextPlain,
    HCTTextXml,

/* image/? */
    HCTImageGif,
    HCTImageJpeg,
    HCTImagePng,

/* application/? */
    HCTApplicationXhtmlXml,
    HCTApplicationXml,
    HCTApplicationAtomXml,
    HCTApplicationJson,
    HCTApplicationPdf,
    HCTApplicationMsword,
    HCTApplicationOctetStream,
    HCTApplicationXWwwFormUrlencoded,

/* multipart/? */
    HCTMutipartFormData,

/* end of all content type */
    HCTUnknown
} http_content_type_e;

typedef struct {

    struct {
        http_method_e method;
        char header[8096];
        char * body;
    } req;

    struct {
        http_status_e status;
        char header[8096];
        char * body;
    } res;

    char msg[64]; /* feedback message for handler function */
} http_s;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MYTH_EPIC_HTTP */
