/* ST5004CEM - Task 4: Network Programming and IPC
 * Stage 3: Client for structured protocol (LOGIN / MSG / QUIT)
 *
 * Compile: gcc -o client client.c
 * Run:     ./client
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
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server on port %d.\n", PORT);
    printf("Commands: LOGIN <user> <pass> | MSG <text> | QUIT\n");
    printf("(try LOGIN admin admin123)\n");

    while (1) {
        printf("> ");
        if (!fgets(buffer, BUFFER_SIZE, stdin)) break;

        send(sock_fd, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(sock_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            printf("Server closed the connection.\n");
            break;
        }
        printf("%s", buffer);

        if (strncmp(buffer, "OK: Goodbye", 11) == 0) break;
    }

    close(sock_fd);
    return 0;
}
