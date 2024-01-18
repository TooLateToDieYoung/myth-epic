/* C89 Std. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* C11 Std. */
#include <threads.h>

/* UNIX */
#include <unistd.h>
#include <arpa/inet.h>

int handleClient(void *client_socket_ptr) {
    int client_socket = *((int *)client_socket_ptr);
    free(client_socket_ptr);

    // TODO

    close(client_socket);
    printf("Connection closed.\n");

    return 0;
}

void startServer(int port, int max_connections) {
    int server_socket, *client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    thrd_t thread_id;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, max_connections) == -1) {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d with a maximum of %d connections...\n", port, max_connections);

    while (1) {
        if ((client_socket = (int *)malloc(sizeof(int))) == NULL) {
            perror("Error allocating memory");
            exit(EXIT_FAILURE);
        }

        if ((*client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len)) == -1) {
            perror("Error accepting connection");
            free(client_socket);
            continue;
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        if (thrd_create(&thread_id, (thrd_start_t)handleClient, (void *)client_socket) != thrd_success) {
            perror("Error creating thread");
            free(client_socket);
            continue;
        }

        thrd_join(thread_id, NULL);
    }

    close(server_socket);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <max_connections>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int max_connections = atoi(argv[2]);

    if (port <= 0 || max_connections <= 0) {
        fprintf(stderr, "Invalid port or max_connections value\n");
        exit(EXIT_FAILURE);
    }

    startServer(port, max_connections);

    return 0;
}
