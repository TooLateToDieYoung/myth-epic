/* C89 Std. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* C11 Std. */
#include <threads.h>

/* UNIX */
#include <unistd.h>
#include <arpa/inet.h>

static int handler(void *param)
{
    if (NULL == param)
    {
        return -1;
    }

    const int clientSocket = *(int *)param;
    free(param);

    const char * const content = "<html><head><title>MythEpic</title></head><body><div>Hello, World!</div></body></html>";

    char response[256] = {0};
    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n%s", strlen(content), content);

    send(clientSocket, response, sizeof(response), 0);

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

        ret = thrd_create(&threadId, (thrd_start_t)handler, (void *)clientSocket);
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
