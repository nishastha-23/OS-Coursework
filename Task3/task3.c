/* ST5004CEM - Task 3: Secure File Management System
 * Stage 4: Encryption / Decryption (XOR cipher)
 *
 * Compile: gcc -o task3 task3.c
 * Run:     ./task3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS   20
#define MAX_FILES   50
#define VAULT_DIR   "vault"

typedef struct {
    char username[50];
    char passhash[64];
    char group[30];
} User;

typedef struct {
    char filename[100];
    char owner[50];
    char group[30];
    char perms[10];
    int  encrypted;
} FileMeta;

User users[MAX_USERS];
int user_count = 0;

FileMeta files[MAX_FILES];
int file_count = 0;

User *current_user = NULL;

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

void load_files(void) {
    FILE *f = fopen("files.meta", "r");
    if (!f) return;
    file_count = 0;
    while (file_count < MAX_FILES &&
           fscanf(f, "%99[^:]:%49[^:]:%29[^:]:%9[^:]:%d\n",
                  files[file_count].filename,
                  files[file_count].owner,
                  files[file_count].group,
                  files[file_count].perms,
                  &files[file_count].encrypted) == 5) {
        file_count++;
    }
    fclose(f);
}

void save_files(void) {
    FILE *f = fopen("files.meta", "w");
    if (!f) return;
    for (int i = 0; i < file_count; i++) {
        fprintf(f, "%s:%s:%s:%s:%d\n",
                files[i].filename, files[i].owner, files[i].group,
                files[i].perms, files[i].encrypted);
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

int check_permission(FileMeta *fm, char op) {
    int offset;
    if (strcmp(fm->owner, current_user->username) == 0) {
        offset = 0;
    } else if (strcmp(fm->group, current_user->group) == 0) {
        offset = 3;
    } else {
        offset = 6;
    }
    int pos = (op == 'r') ? 0 : (op == 'w') ? 1 : 2;
    return fm->perms[offset + pos] == op;
}

FileMeta *find_file(const char *filename) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].filename, filename) == 0) return &files[i];
    }
    return NULL;
}

void xor_encrypt_decrypt(char *filepath, const char *key) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *data = malloc(size);
    fread(data, 1, size, f);
    fclose(f);

    int klen = strlen(key);
    for (long i = 0; i < size; i++) {
        data[i] ^= key[i % klen];
    }

    f = fopen(filepath, "wb");
    fwrite(data, 1, size, f);
    fclose(f);
    free(data);
}
void create_file_op(void) {
    if (file_count >= MAX_FILES) { printf("File limit reached.\n"); return; }
    char filename[100], perms[10];
    printf("Filename: ");
    scanf("%99s", filename);

    if (find_file(filename)) { printf("File already exists.\n"); return; }

    printf("Permissions (9 chars, e.g. rwxr-x---): ");
    scanf("%9s", perms);
    if (strlen(perms) != 9) {
        printf("Invalid permission string, using default rwxr-----\n");
        strcpy(perms, "rwxr-----");
    }

    char filepath[150];
    snprintf(filepath, sizeof(filepath), "%s/%s", VAULT_DIR, filename);
    FILE *f = fopen(filepath, "w");
    if (!f) { printf("Error creating file.\n"); return; }
    fclose(f);

    strcpy(files[file_count].filename, filename);
    strcpy(files[file_count].owner, current_user->username);
    strcpy(files[file_count].group, current_user->group);
    strcpy(files[file_count].perms, perms);
    files[file_count].encrypted = 0;
    file_count++;
    save_files();

    printf("File '%s' created (owner=%s, perms=%s).\n", filename, current_user->username, perms);
}

void write_file_op(void) {
    char filename[100];
    printf("Filename: ");
    scanf("%99s", filename);
    FileMeta *fm = find_file(filename);
    if (!fm) { printf("File not found.\n"); return; }
    if (!check_permission(fm, 'w')) { printf("PERMISSION DENIED: no write access.\n"); return; }
    if (fm->encrypted) { printf("File is encrypted. Decrypt it first before writing.\n"); return; }

    char content[1000];
    printf("Enter content to write (single line): ");
    getchar();
    fgets(content, sizeof(content), stdin);

    char filepath[150];
    snprintf(filepath, sizeof(filepath), "%s/%s", VAULT_DIR, filename);
    FILE *f = fopen(filepath, "w");
    fputs(content, f);
    fclose(f);

    printf("Content written to '%s'.\n", filename);
}

void read_file_op(void) {
    char filename[100];
    printf("Filename: ");
    scanf("%99s", filename);
    FileMeta *fm = find_file(filename);
    if (!fm) { printf("File not found.\n"); return; }
    if (!check_permission(fm, 'r')) { printf("PERMISSION DENIED: no read access.\n"); return; }
    if (fm->encrypted) { printf("File is encrypted. Decrypt it first to read plaintext.\n"); return; }

    char filepath[150];
    snprintf(filepath, sizeof(filepath), "%s/%s", VAULT_DIR, filename);
    FILE *f = fopen(filepath, "r");
    if (!f) { printf("Error opening file.\n"); return; }

    char line[1000];
    printf("--- Contents of %s ---\n", filename);
    while (fgets(line, sizeof(line), f)) printf("%s", line);
    printf("--- End ---\n");
    fclose(f);
}

void delete_file_op(void) {
    char filename[100];
    printf("Filename: ");
    scanf("%99s", filename);
    FileMeta *fm = find_file(filename);
    if (!fm) { printf("File not found.\n"); return; }
    if (!check_permission(fm, 'w')) { printf("PERMISSION DENIED: only write-permitted users may delete.\n"); return; }

    char filepath[150];
    snprintf(filepath, sizeof(filepath), "%s/%s", VAULT_DIR, filename);
    remove(filepath);

    int idx = fm - files;
    for (int i = idx; i < file_count - 1; i++) files[i] = files[i + 1];
    file_count--;
    save_files();

    printf("File '%s' deleted.\n", filename);
}

void chmod_file_op(void) {
    char filename[100], perms[10];
    printf("Filename: ");
    scanf("%99s", filename);
    FileMeta *fm = find_file(filename);
    if (!fm) { printf("File not found.\n"); return; }
    if (strcmp(fm->owner, current_user->username) != 0) {
        printf("PERMISSION DENIED: only the owner can change permissions.\n");
        return;
    }
    printf("New permissions (9 chars, e.g. rwxr-x---): ");
    scanf("%9s", perms);
    if (strlen(perms) != 9) { printf("Invalid format.\n"); return; }
    strcpy(fm->perms, perms);
    save_files();
    printf("Permissions updated to '%s'.\n", perms);
}

void encrypt_file_op(void) {
    char filename[100], key[50];
    printf("Filename: ");
    scanf("%99s", filename);
    FileMeta *fm = find_file(filename);
    if (!fm) { printf("File not found.\n"); return; }
    if (!check_permission(fm, 'w')) { printf("PERMISSION DENIED.\n"); return; }
    if (fm->encrypted) { printf("File is already encrypted.\n"); return; }

    printf("Encryption key: ");
    scanf("%49s", key);

    char filepath[150];
    snprintf(filepath, sizeof(filepath), "%s/%s", VAULT_DIR, filename);
    xor_encrypt_decrypt(filepath, key);
    fm->encrypted = 1;
    save_files();

    printf("File '%s' encrypted.\n", filename);
}

void decrypt_file_op(void) {
    char filename[100], key[50];
    printf("Filename: ");
    scanf("%99s", filename);
    FileMeta *fm = find_file(filename);
    if (!fm) { printf("File not found.\n"); return; }
    if (!check_permission(fm, 'w')) { printf("PERMISSION DENIED.\n"); return; }
    if (!fm->encrypted) { printf("File is not encrypted.\n"); return; }

    printf("Decryption key: ");
    scanf("%49s", key);

    char filepath[150];
    snprintf(filepath, sizeof(filepath), "%s/%s", VAULT_DIR, filename);
    xor_encrypt_decrypt(filepath, key);
    fm->encrypted = 0;
    save_files();

    printf("File '%s' decrypted (if the key was correct, it is now readable).\n", filename);
}

void list_files(void) {
    printf("\n%-20s %-12s %-10s %-12s %-10s\n", "Filename", "Owner", "Group", "Permissions", "Encrypted");
    for (int i = 0; i < file_count; i++) {
        printf("%-20s %-12s %-10s %-12s %-10s\n",
               files[i].filename, files[i].owner, files[i].group,
               files[i].perms, files[i].encrypted ? "yes" : "no");
    }
    printf("\n");
}

int main(void) {
    system("mkdir -p " VAULT_DIR);
    load_users();
    load_files();

    printf("###############################################################\n");
    printf("# ST5004CEM Task 3 - Secure File Management System            #\n");
    printf("# Stage 4: Encryption                                         #\n");
    printf("###############################################################\n");

    while (!current_user) {
        printf("\n1. Register\n2. Login\n3. Exit\nChoice: ");
        int choice;
        scanf("%d", &choice);
        if (choice == 1) register_user();
        else if (choice == 2) login_user();
        else if (choice == 3) { printf("Goodbye.\n"); return 0; }
    }

    int choice;
    do {
        printf("\n===== MENU (logged in as %s) =====\n", current_user->username);
        printf("1. Create file\n2. Write to file\n3. Read file\n4. Delete file\n");
        printf("5. Change permissions\n6. Encrypt file\n7. Decrypt file\n8. List files\n0. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: create_file_op(); break;
            case 2: write_file_op(); break;
            case 3: read_file_op(); break;
            case 4: delete_file_op(); break;
            case 5: chmod_file_op(); break;
            case 6: encrypt_file_op(); break;
            case 7: decrypt_file_op(); break;
            case 8: list_files(); break;
            case 0: printf("Logging out. Goodbye.\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}
