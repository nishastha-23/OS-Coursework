#include <stdio.h>

#define PAGE_SIZE_KB   4
#define NUM_FRAMES     3
#define REF_LENGTH     12

int reference_string[REF_LENGTH] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};

void run_fifo(void) {
    int frames[NUM_FRAMES];
    for (int i = 0; i < NUM_FRAMES; i++) frames[i] = -1;

    int oldest_index = 0;
    int filled = 0;
    int faults = 0, hits = 0;

    printf("\n================ FIFO PAGE REPLACEMENT ================\n");
    printf("Page Size = %d KB | Frames = %d\n\n", PAGE_SIZE_KB, NUM_FRAMES);

    for (int i = 0; i < REF_LENGTH; i++) {
        int page = reference_string[i];
        int found = 0;

        for (int f = 0; f < NUM_FRAMES; f++) {
            if (frames[f] == page) { found = 1; break; }
        }

        if (found) {
            hits++;
            printf("Request %2d: page %d -> HIT   | Frames: ", i + 1, page);
        } else {
            faults++;
            frames[oldest_index] = page;
            oldest_index = (oldest_index + 1) % NUM_FRAMES;
            if (filled < NUM_FRAMES) filled++;
            printf("Request %2d: page %d -> FAULT | Frames: ", i + 1, page);
        }

        for (int f = 0; f < NUM_FRAMES; f++) {
            if (frames[f] == -1) printf("_ ");
            else printf("%d ", frames[f]);
        }
        printf("\n");
    }

    printf("\nTotal Requests = %d\n", REF_LENGTH);
    printf("Page Faults    = %d\n", faults);
    printf("Page Hits      = %d\n", hits);
    printf("Hit Ratio      = %.2f%%\n", (hits * 100.0) / REF_LENGTH);
    printf("Miss Ratio     = %.2f%%\n", (faults * 100.0) / REF_LENGTH);
}

void run_lru(void) {
    int frames[NUM_FRAMES];
    int last_used[NUM_FRAMES];
    for (int i = 0; i < NUM_FRAMES; i++) { frames[i] = -1; last_used[i] = -1; }

    int faults = 0, hits = 0;

    printf("\n================ LRU PAGE REPLACEMENT ================\n");
    printf("Page Size = %d KB | Frames = %d\n\n", PAGE_SIZE_KB, NUM_FRAMES);

    for (int i = 0; i < REF_LENGTH; i++) {
        int page = reference_string[i];
        int found = -1;

        for (int f = 0; f < NUM_FRAMES; f++) {
            if (frames[f] == page) { found = f; break; }
        }

        if (found != -1) {
            hits++;
            last_used[found] = i;
            printf("Request %2d: page %d -> HIT   | Frames: ", i + 1, page);
        } else {
            faults++;
            int target = -1;
            for (int f = 0; f < NUM_FRAMES; f++) {
                if (frames[f] == -1) { target = f; break; }
            }
            if (target == -1) {
                target = 0;
                for (int f = 1; f < NUM_FRAMES; f++) {
                    if (last_used[f] < last_used[target]) target = f;
                }
            }
            frames[target] = page;
            last_used[target] = i;
            printf("Request %2d: page %d -> FAULT | Frames: ", i + 1, page);
        }

        for (int f = 0; f < NUM_FRAMES; f++) {
            if (frames[f] == -1) printf("_ ");
            else printf("%d ", frames[f]);
        }
        printf("\n");
    }

    printf("\nTotal Requests = %d\n", REF_LENGTH);
    printf("Page Faults    = %d\n", faults);
    printf("Page Hits      = %d\n", hits);
    printf("Hit Ratio      = %.2f%%\n", (hits * 100.0) / REF_LENGTH);
    printf("Miss Ratio     = %.2f%%\n", (faults * 100.0) / REF_LENGTH);
}

int main(void) {
    printf("###############################################################\n");
    printf("# ST5004CEM Task 2 - Memory Management Simulation             #\n");
    printf("###############################################################\n");

    printf("\nPage Reference String: ");
    for (int i = 0; i < REF_LENGTH; i++) printf("%d ", reference_string[i]);
    printf("\n");

    run_fifo();
    run_lru();

    printf("\n================ COMPARISON SUMMARY ================\n");
    printf("Both algorithms were tested on the same reference string\n");
    printf("with %d frames available. See fault counts above to compare\n", NUM_FRAMES);
    printf("FIFO vs LRU performance.\n");

    return 0;
}
