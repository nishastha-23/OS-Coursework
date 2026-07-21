/* ST5004CEM - Task 4: Network Programming and IPC
 * Stage 3: Structured Protocol (LOGIN / MSG / QUIT)
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

/* Simple protocol handler: parses commands sent by the client and
 * validates them before acting. This gives us both a "protocol"
 * and basic "data validation" as required by the brief. */
void process_command(char *buffer, char *response, int *logged_in, char *username) {
    /* Strip trailing newline for cleaner parsing */
    buffer[strcspn(buffer, "\n")] = 0;

    if (strncmp(buffer, "LOGIN ", 6) == 0) {
        char user[100], pass[100];
        if (sscanf(buffer + 6, "%99s %99s", user, pass) != 2) {
            strcpy(response, "ERROR: LOGIN requires <username> <password>\n");
            return;
        }
        /* NOTE: hardcoded demo credentials - Stage 4 will connect this
         * to a real user database with hashed passwords. */
        if (strcmp(user, "admin") == 0 && strcmp(pass, "admin123") == 0) {
            *logged_in = 1;
            strcpy(username, user);
            sprintf(response, "OK: Login successful, welcome %s\n", user);
        } else {
            strcpy(response, "ERROR: Invalid credentials\n");
        }
    }
    else if (strncmp(buffer, "MSG ", 4) == 0) {
        if (!*logged_in) {
            strcpy(response, "ERROR: You must LOGIN before sending messages\n");
            return;
        }
        sprintf(response, "OK: [%s] %s\n", username, buffer + 4);
    }
    else if (strcmp(buffer, "QUIT") == 0) {
        strcpy(response, "OK: Goodbye\n");
    }
    else {
        strcpy(response, "ERROR: Unknown command. Use LOGIN, MSG, or QUIT\n");
    }
}

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int logged_in = 0;
    char username[100] = "";

    pthread_mutex_lock(&count_lock);
    client_count++;
    int my_id = client_count;
    pthread_mutex_unlock(&count_lock);

    printf("[Client #%d] connected\n", my_id);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            printf("[Client #%d] disconnected.\n", my_id);
            break;
        }

        printf("[Client #%d] Command: %s", my_id, buffer);

        memset(response, 0, BUFFER_SIZE);
        process_command(buffer, response, &logged_in, username);
        send(client_fd, response, strlen(response), 0);

        if (strncmp(buffer, "QUIT", 4) == 0) {
            printf("[Client #%d] sent QUIT.\n", my_id);
            break;
        }
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

    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d... (protocol: LOGIN / MSG / QUIT)\n", PORT);

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
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
