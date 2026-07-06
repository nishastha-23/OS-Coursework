/* ST5004CEM - Task 1 Part A: Producer-Consumer with Mutex + Condition Variables
 * Compile: gcc -o task1a task1a.c -lpthread
 * Run:     ./task1a
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE   5
#define ITEMS_PER_PRODUCER 6

typedef struct {
    int buffer[BUFFER_SIZE];
    int count;
    int in;
    int out;
    pthread_mutex_t lock;
    pthread_cond_t  not_full;
    pthread_cond_t  not_empty;
} SharedBuffer;

static SharedBuffer shared_buf;
static int total_produced = 0;
static int total_consumed = 0;

void buffer_init(SharedBuffer *b) {
    b->count = 0;
    b->in = 0;
    b->out = 0;
    pthread_mutex_init(&b->lock, NULL);
    pthread_cond_init(&b->not_full, NULL);
    pthread_cond_init(&b->not_empty, NULL);
}

void buffer_destroy(SharedBuffer *b) {
    pthread_mutex_destroy(&b->lock);
    pthread_cond_destroy(&b->not_full);
    pthread_cond_destroy(&b->not_empty);
}

typedef struct {
    int id;
} ProducerArg;

void *producer_thread(void *arg) {
    ProducerArg *pa = (ProducerArg *)arg;
    int id = pa->id;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = (id * 100) + i;

        pthread_mutex_lock(&shared_buf.lock);

        while (shared_buf.count == BUFFER_SIZE) {
            pthread_cond_wait(&shared_buf.not_full, &shared_buf.lock);
        }

        shared_buf.buffer[shared_buf.in] = item;
        shared_buf.in = (shared_buf.in + 1) % BUFFER_SIZE;
        shared_buf.count++;
        total_produced++;

        printf("[Producer %d] produced item %d (buffer count = %d)\n",
               id, item, shared_buf.count);

        pthread_cond_signal(&shared_buf.not_empty);
        pthread_mutex_unlock(&shared_buf.lock);

        usleep(50000);
    }
    return NULL;
}

void *consumer_thread(void *arg) {
    (void)arg;
    int expected_total = ITEMS_PER_PRODUCER * 2;

    while (total_consumed < expected_total) {
        pthread_mutex_lock(&shared_buf.lock);

        while (shared_buf.count == 0) {
            pthread_cond_wait(&shared_buf.not_empty, &shared_buf.lock);
        }

        int item = shared_buf.buffer[shared_buf.out];
        shared_buf.out = (shared_buf.out + 1) % BUFFER_SIZE;
        shared_buf.count--;
        total_consumed++;

        printf("    [Consumer] consumed item %d (buffer count = %d)\n",
               item, shared_buf.count);

        pthread_cond_signal(&shared_buf.not_full);
        pthread_mutex_unlock(&shared_buf.lock);

        usleep(70000);
    }
    return NULL;
}

int main(void) {
    printf("================ TASK 1 - PART A: PRODUCER-CONSUMER ================\n");
    buffer_init(&shared_buf);

    pthread_t producers[2], consumer;
    ProducerArg args[2] = { {1}, {2} };

    pthread_create(&producers[0], NULL, producer_thread, &args[0]);
    pthread_create(&producers[1], NULL, producer_thread, &args[1]);
    pthread_create(&consumer,     NULL, consumer_thread, NULL);

    pthread_join(producers[0], NULL);
    pthread_join(producers[1], NULL);
    pthread_join(consumer,     NULL);

    printf("Total produced = %d, Total consumed = %d\n",
           total_produced, total_consumed);
    printf("(3 threads used: 2 producers + 1 consumer, synchronized via mutex + condvars)\n");

    buffer_destroy(&shared_buf);
    return 0;
}
