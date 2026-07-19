/* ST5004CEM - Task 3: Secure File Management System
 * Stage 1: User Authentication (Register / Login)
 *
 * Compile: gcc -o task3 task3.c
 * Run:     ./task3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS   20

typedef struct {
    char username[50];
    char passhash[64];
    char group[30];
} User;

User users[MAX_USERS];
int user_count = 0;

User *current_user = NULL;

/* NOTE: this is a basic additive hash for demonstration purposes only.
 * A real system should use a proper algorithm (e.g. bcrypt, SHA-256 with
 * salting) - this limitation will be discussed in the security analysis
 * report as a known weakness of this design. */
void hash_password(const char *password, char *out_hash) {
    unsigned long hash = 5381;
    for (int i = 0; password[i] != '\0'; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)password[i];
    }
    sprintf(out_hash, "%lu", hash);
}

void load_users(void) {
    FILE *f = fopen("users.dat", "r");
    if (!f) return;
    user_count = 0;
    while (user_count < MAX_USERS &&
           fscanf(f, "%49[^:]:%63[^:]:%29[^\n]\n",
                  users[user_count].username,
                  users[user_count].passhash,
                  users[user_count].group) == 3) {
        user_count++;
    }
    fclose(f);
}

void save_users(void) {
    FILE *f = fopen("users.dat", "w");
    if (!f) return;
    for (int i = 0; i < user_count; i++) {
        fprintf(f, "%s:%s:%s\n", users[i].username, users[i].passhash, users[i].group);
    }
    fclose(f);
}

void register_user(void) {
    if (user_count >= MAX_USERS) { printf("User limit reached.\n"); return; }
    char username[50], password[50], group[30];
    printf("New username: ");
    scanf("%49s", username);

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            printf("Username already exists.\n");
            return;
        }
    }

    printf("New password: ");
    scanf("%49s", password);
    printf("Group (e.g. staff, students): ");
    scanf("%29s", group);

    strcpy(users[user_count].username, username);
    hash_password(password, users[user_count].passhash);
    strcpy(users[user_count].group, group);
    user_count++;
    save_users();
    printf("User '%s' registered successfully.\n", username);
}

int login_user(void) {
    char username[50], password[50], hash[64];
    printf("Username: ");
    scanf("%49s", username);
    printf("Password: ");
    scanf("%49s", password);
    hash_password(password, hash);

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].passhash, hash) == 0) {
            current_user = &users[i];
            printf("Login successful. Welcome, %s (group: %s)\n",
                   current_user->username, current_user->group);
            return 1;
        }
    }
    printf("Invalid username or password.\n");
    return 0;
}

int main(void) {
    load_users();

    printf("###############################################################\n");
    printf("# ST5004CEM Task 3 - Secure File Management System            #\n");
    printf("# Stage 1: Authentication                                     #\n");
    printf("###############################################################\n");

    while (!current_user) {
        printf("\n1. Register\n2. Login\n3. Exit\nChoice: ");
        int choice;
        scanf("%d", &choice);
        if (choice == 1) register_user();
        else if (choice == 2) login_user();
        else if (choice == 3) { printf("Goodbye.\n"); return 0; }
    }

    printf("\nLogged in as %s. (File operations will be added in the next stage.)\n",
           current_user->username);
    return 0;
}
