/* ST5004CEM - Task 4: Network Programming and IPC
 * Stage 2: Multi-Client TCP Server (using threads)
 *
 * Compile: gcc -o server server.c -lpthread
 * Run:     ./server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int client_count = 0;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

/* Each connected client is handled in its own thread, so the server
 * can serve multiple clients concurrently instead of one at a time. */
void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];

    pthread_mutex_lock(&count_lock);
    client_count++;
    int my_id = client_count;
    pthread_mutex_unlock(&count_lock);

    printf("[Client #%d] connected on thread %lu\n", my_id, (unsigned long)pthread_self());

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            printf("[Client #%d] disconnected.\n", my_id);
            break;
        }
        printf("[Client #%d] Received: %s", my_id, buffer);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    close(client_fd);
    return NULL;
}

int main(void) {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    /* backlog of 10 allows several pending connections to queue up */
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d... (supports multiple clients)\n", PORT);

    /* Main loop: keep accepting new clients forever, spawning a thread
     * for each one so the server never blocks on a single client. */
    while (1) {
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (*client_fd < 0) {
            perror("accept failed");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid); /* auto-cleanup when thread finishes */
    }

    close(server_fd);
    return 0;
}
