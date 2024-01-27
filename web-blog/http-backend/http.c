/* C89 Std. */
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* C99 Std. */
#include <stdbool.h>

/* C11 Std. */
#include <threads.h>

/* UNIX */
#include <unistd.h>
#include <arpa/inet.h>

/* Myth Epic Lib. */
#include "http.h"
#include "api.h"
#include "list.h"

#define BUFFER_SIZE 128

static bool _httpIsWithoutBodyMethod(const http_method_e method)
{
    return (
        method == HMGet ||
        method == HMHead ||
        method == HMOption ||
        method == HMDelete ||
        method == HMTrace
    );
}

static size_t _httpHeaderStringify(void * val, char * buffer, size_t size)
{
    return (size_t)snprintf(buffer, size, "%s\r\n", (char *)val);
}

static int _httpRecvUntil(int srcSocket, char dstbuf[], size_t dstBufLen, const char * const dstEndStr)
{
    for ( size_t idx = 0; dstBufLen > idx; ++idx )
    {
        if ( 1 == recv(srcSocket, &dstbuf[idx], sizeof(char), 0) )
        {
            if ( NULL != strstr(dstbuf, dstEndStr) )
            {
                return idx;
            }
        }
        else
        {
            break;
        }
    }

    return -1;
}

static http_method_e _httpMethod(const char * const method)
{
    if ( NULL != method )
    {
        if ( 0 == strcmp(method, "GET") ) { return HMGet; }
        if ( 0 == strcmp(method, "PATCH") ) { return HMPatch; }
        if ( 0 == strcmp(method, "PUT") ) { return HMPut; }
        if ( 0 == strcmp(method, "POST") ) { return HMPost; }
        if ( 0 == strcmp(method, "DELETE") ) { return HMDelete; }
        if ( 0 == strcmp(method, "OPTION") ) { return HMOption; }
        if ( 0 == strcmp(method, "HEAD") ) { return HMHead; }
        if ( 0 == strcmp(method, "CONNECT") ) { return HMConnect; }
        if ( 0 == strcmp(method, "TRACE") ) { return HMTrace; }
    }

    return HMUnknown;
}

static char * _httpStatus(http_status_e status)
{
    switch (status)
    {
        case HSOk: return "OK";
        case HSNotFound: return "Not Found";

        // TODO
        
        default: break;
    }

    return "";
}

static int _httpEntrance(void *param)
{
    if (NULL == param)
    {
        return -1;
    }

    const int clientSocket = *(int *)param;
    free(param);

    int ret = -1;
    char path[BUFFER_SIZE] = {0};
    http_s http = {0};

    /* get request line */
    char line[BUFFER_SIZE] = {0};
    ret = _httpRecvUntil(clientSocket, line, sizeof(line), "\r\n");
    if ( ret > 0 )
    {
        fprintf(stdout, "[INFO] request line: %s", line);

        const char * part = NULL;

        part = strtok(line, " ");
        if ( NULL != part )
        {
            http.req.method = _httpMethod(part);
            if ( HMUnknown == http.req.method )
            {
                // ! catch error
            }
        }

        part = strtok(NULL, " ");
        if ( NULL != part )
        {
            const char * const queryPtr = strchr(part, '?');
            const size_t pathLen = ( NULL != queryPtr ) ? ( queryPtr - part ) : strlen(part) ;
            const size_t queryLen = ( NULL != queryPtr ) ? ( strlen(part) - pathLen - 1 ) : ( 0 ) ;
            if ( sizeof(path) > pathLen && sizeof(http.query) > queryLen )
            {
                if ( NULL == queryPtr )
                {
                    sscanf(part, "%s", path);
                }
                else
                {
                    sscanf(part, "%[^?]%s", path, http.query);
                }
            }
            else
            {
                // ! catch error
            }
        }

        part = strtok(NULL, " ");
#if 0
        if ( NULL != part )
        {
            fprintf(stdout, "[INFO] http request version: %s\n", part);
        }
#endif
    }
    else
    {
        // ! catch error
    }

    /* get request headers */
    ret = _httpRecvUntil(clientSocket, http.req.header, sizeof(http.req.header), "\r\n\r\n");
    if ( ret > 0 )
    {
        fprintf(stdout, "[INFO] request header: %s", http.req.header);

        // ? do something check
    }
    else
    {
        // ! catch error
    }

    /* get request body */
    char * endPtr = NULL;
    size_t contentLength = 0;
    if ( _httpIsWithoutBodyMethod(http.req.method) )
    {
        // ? do something and don't care about the body
    }
    else if ( NULL != strstr(http.req.header, "Content-Type:") )
    {
        const char * const headerPtr = strstr(http.req.header, "Content-Type:"); 
        contentLength = (size_t)strtoull(headerPtr + strlen("Content-Type:"), &endPtr, 10);
        if ( NULL == endPtr || isgraph(*endPtr) )
        { 
            // ! catch error
        }

        http.req.body = (char *)calloc(contentLength, sizeof(char));
        if ( NULL != http.req.body )
        {
            ret = recv(clientSocket, http.req.body, contentLength, 0);
        }
        else
        {
            // ! catch error
        }
    }
    else if ( NULL != strstr(http.req.header, "Transfer-Encoding:") )
    {
        list_s * const list = listMake(free);
        if ( NULL != list )
        {
            char * chunkData = NULL;
            char chunkInfo[BUFFER_SIZE] = {0};
            size_t chunkSize = 0;
            do {
                memset(chunkInfo, 0, sizeof(chunkInfo));

                ret = _httpRecvUntil(clientSocket, chunkInfo, sizeof(chunkInfo), "\r\n");
                if ( ret > 0 )
                {
                    chunkSize = strtoull(chunkInfo, &endPtr, 16);
                    if ( NULL == endPtr || isgraph(*endPtr) )
                    { 
                        // ! catch error
                    }
                }
                else
                {
                    // ! catch error
                }

                chunkData = (char *)calloc(chunkSize, sizeof(char));
                if ( NULL == chunkData )
                {
                    // ! catch error
                }

                ret = recv(clientSocket, chunkData, chunkSize, 0);
                if ( ret > 0 )
                {
                    listInsert(list, ~0, chunkData);
                }
                else
                {
                    // ! catch error
                    free(chunkData);
                }
                
            } while ( 0 != chunkSize );

            contentLength = 1 + listStringify(list, NULL, 0, "", _httpHeaderStringify);
            if ( contentLength > 0 )
            {
                http.req.body = (char *)calloc(contentLength, sizeof(char));
                if ( NULL != http.req.body )
                {
                    listStringify(list, http.req.body, contentLength, "", _httpHeaderStringify);
                }
                else
                {
                    // ! catch error
                }
                listFree(list);
            }
        }
        else
        {
            // ! catch error
        }
    }
    else
    {
        // TODO: how to recv an undefined length body
    }

    /* looking for the handler */
    for ( size_t idx = 0; NULL != api[idx].path; ++idx )
    {
        if ( 0 == strcmp(path, api[idx].path) )
        {
assert(api[idx].handler);

            ret = api[idx].handler(&http);
            if ( NULL != http.req.body )
            {
                free(http.req.body);
                http.req.body = NULL;
            }
            break;
        }
    }

    if ( 0 == ret )
    {
        /* send response line */
        memset(line, 0, sizeof(line));
        snprintf(line, sizeof(line), "HTTP/1.1 %d %s\r\n", http.res.status, _httpStatus(http.res.status));
        send(clientSocket, line, strlen(line), 0);

        /* send response headers */
        if ( 0 != strlen(http.res.header) )
        {
            send(clientSocket, http.res.header, strlen(http.res.header), 0);
        }

        send(clientSocket, "\r\n", strlen("\r\n"), 0);

        /* send response body if needed */
        if ( NULL != http.res.body )
        {
            send(clientSocket, http.res.body, strlen(http.res.body), 0);
            free(http.res.body);
            http.res.body = NULL;
        }
    }
    else
    {
        fprintf(stderr, "[ERROR] %s\n", http.msg);
        // ! catch error
    }

    close(clientSocket);
    fprintf(stdout, "[INFO] connection closed\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int ret = 0;

/* check parameters */
    if (3 != argc)
    {
        fprintf(stderr, "Usage: %s <listen_port> <max_connections>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const int listenPort = atoi(argv[1]);
    const int maxConnections = atoi(argv[2]);
    if (0 >= listenPort || 0 >= maxConnections)
    {
        fprintf(stderr, "[ERROR] invalid listen_port or max_connections value\n");
        exit(EXIT_FAILURE);
    }

/* prepare server socket */
    const int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == serverSocket)
    {
        fprintf(stderr, "[ERROR] error creating socket > %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverSockAddr = {0};
    serverSockAddr.sin_family = AF_INET;
    serverSockAddr.sin_port = htons(listenPort);
    serverSockAddr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(serverSocket, (struct sockaddr *)&serverSockAddr, sizeof(serverSockAddr));
    if (-1 == ret)
    {
        close(serverSocket);
        fprintf(stderr, "[ERROR] error binding socket > %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    ret = listen(serverSocket, maxConnections);
    if (-1 == ret)
    {
        close(serverSocket);
        fprintf(stderr, "[ERROR] error listening on socket > %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "[INFO] server listening on port %d with a maximum of %d connections\n", listenPort, maxConnections);

/* trap and deal with client sockets */
    int *clientSocket = NULL;
    struct sockaddr_in clientSockAddr = {0};
    socklen_t clientSockAddrLen = (socklen_t)sizeof(clientSockAddr);
    thrd_t threadId = 0;
    for (;;)
    {
        clientSocket = (int *)calloc(1, sizeof(int));
        if (NULL == clientSocket)
        {
            fprintf(stderr, "[ERROR] error allocating memory for client socket > %s\n", strerror(errno));
            break;
        }

        *clientSocket = accept(serverSocket, (struct sockaddr *)&clientSockAddr, &clientSockAddrLen);
        if (-1 == *clientSocket)
        {
            free(clientSocket);
            fprintf(stderr, "[ERROR] error accepting connection > %s\n", strerror(errno));
            continue;
        }

        fprintf(stdout, "[INFO] connection accepted from %s:%d\n", inet_ntoa(clientSockAddr.sin_addr), ntohs(clientSockAddr.sin_port));

        ret = thrd_create(&threadId, (thrd_start_t)_httpEntrance, (void *)clientSocket);
        if (thrd_success != ret)
        {
            free(clientSocket);
            fprintf(stderr, "[ERROR] error creating thread > %s\n", strerror(errno));
            continue;
        }

        thrd_detach(threadId); // ? free(clientSocket) in handler() function
    }

    close(serverSocket);
    fprintf(stdout, "[INFO] server closed\n");
    exit(EXIT_SUCCESS);
}
