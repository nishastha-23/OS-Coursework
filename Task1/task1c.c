/* ST5004CEM - Task 1 Part C: Deadlock Demonstration and Prevention
 * Compile: gcc -o task1c task1c.c -lpthread
 * Run:     ./task1c
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static pthread_mutex_t lock_A = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lock_B = PTHREAD_MUTEX_INITIALIZER;

/* Both threads lock A before B - this consistent ordering removes the
 * "circular wait" condition, which is one of the four necessary
 * conditions for deadlock to occur. This is how the deadlock is
 * PREVENTED, not just avoided by luck. */

void *safe_thread1(void *arg) {
    (void)arg;
    printf("[Thread-1] locking A...\n");
    pthread_mutex_lock(&lock_A);
    usleep(100000);
    printf("[Thread-1] locking B...\n");
    pthread_mutex_lock(&lock_B);

    printf("[Thread-1] got both locks, working...\n");
    usleep(50000);

    pthread_mutex_unlock(&lock_B);
    pthread_mutex_unlock(&lock_A);
    printf("[Thread-1] done, released both locks.\n");
    return NULL;
}

void *safe_thread2(void *arg) {
    (void)arg;
    printf("[Thread-2] locking A...\n");
    pthread_mutex_lock(&lock_A);   /* SAME order as Thread-1: A before B */
    usleep(100000);
    printf("[Thread-2] locking B...\n");
    pthread_mutex_lock(&lock_B);

    printf("[Thread-2] got both locks, working...\n");
    usleep(50000);

    pthread_mutex_unlock(&lock_B);
    pthread_mutex_unlock(&lock_A);
    printf("[Thread-2] done, released both locks.\n");
    return NULL;
}

int main(void) {
    printf("================ TASK 1 - PART C: DEADLOCK PREVENTION ================\n");
    printf("Both threads acquire locks in the SAME order (A, then B).\n");
    printf("This breaks the 'circular wait' condition required for deadlock.\n\n");

    pthread_t t1, t2;
    pthread_create(&t1, NULL, safe_thread1, NULL);
    pthread_create(&t2, NULL, safe_thread2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("\nBoth threads completed successfully - no deadlock occurred.\n");
    return 0;
}
