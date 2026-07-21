/* ST5004CEM - Task 4: Network Programming and IPC
 * Stage 1: Basic TCP Server (single client, echo)
 *
 * Compile: gcc -o server server.c
 * Run:     ./server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

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

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (client_fd < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Client connected: %s\n", inet_ntoa(address.sin_addr));

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            printf("Client disconnected.\n");
            break;
        }
        printf("Received: %s", buffer);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}

