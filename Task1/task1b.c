/* ST5004CEM - Task 1 Part B: Round Robin CPU Scheduler Simulation
 * Compile: gcc -o task1b task1b.c
 * Run:     ./task1b
 */

#include <stdio.h>

#define MAX_PROCESSES 5
#define TIME_QUANTUM  2

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
} Process;

int main(void) {
    printf("================ TASK 1 - PART B: ROUND ROBIN SCHEDULER ================\n");

    Process procs[MAX_PROCESSES] = {
        {1, 0, 5, 5, 0, 0, 0},
        {2, 1, 3, 3, 0, 0, 0},
        {3, 2, 8, 8, 0, 0, 0},
        {4, 3, 6, 6, 0, 0, 0},
        {5, 4, 2, 2, 0, 0, 0},
    };
    int n = MAX_PROCESSES;
    int time_elapsed = 0;
    int completed = 0;

    int queue[MAX_PROCESSES * 20];
    int q_front = 0, q_back = 0;
    int in_queue[MAX_PROCESSES] = {0};

    #define ENQUEUE(pidx) do { queue[q_back++] = (pidx); } while (0)

    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time_elapsed && !in_queue[i]) {
            ENQUEUE(i);
            in_queue[i] = 1;
        }
    }

    printf("Gantt Chart:\n| ");

    while (completed < n) {
        if (q_front == q_back) {
            time_elapsed++;
            for (int i = 0; i < n; i++) {
                if (procs[i].arrival_time <= time_elapsed && !in_queue[i] && procs[i].remaining_time > 0) {
                    ENQUEUE(i);
                    in_queue[i] = 1;
                }
            }
            continue;
        }

        int idx = queue[q_front++];
        int run_time = (procs[idx].remaining_time < TIME_QUANTUM)
                        ? procs[idx].remaining_time : TIME_QUANTUM;

        printf("P%d(%d-%d) | ", procs[idx].pid, time_elapsed, time_elapsed + run_time);

        time_elapsed += run_time;
        procs[idx].remaining_time -= run_time;

        for (int i = 0; i < n; i++) {
            if (procs[i].arrival_time <= time_elapsed && !in_queue[i] && procs[i].remaining_time > 0) {
                ENQUEUE(i);
                in_queue[i] = 1;
            }
        }

        if (procs[idx].remaining_time > 0) {
            ENQUEUE(idx);
        } else {
            procs[idx].completion_time = time_elapsed;
            procs[idx].turnaround_time = procs[idx].completion_time - procs[idx].arrival_time;
            procs[idx].waiting_time = procs[idx].turnaround_time - procs[idx].burst_time;
            completed++;
        }
    }
    printf("\n\n");

    double total_wait = 0, total_turnaround = 0;
    printf("%-6s %-10s %-8s %-12s %-10s %-14s\n",
           "PID", "Arrival", "Burst", "Completion", "Waiting", "Turnaround");
    for (int i = 0; i < n; i++) {
        printf("%-6d %-10d %-8d %-12d %-10d %-14d\n",
               procs[i].pid, procs[i].arrival_time, procs[i].burst_time,
               procs[i].completion_time, procs[i].waiting_time, procs[i].turnaround_time);
        total_wait += procs[i].waiting_time;
        total_turnaround += procs[i].turnaround_time;
    }
    printf("\nAverage Waiting Time    = %.2f\n", total_wait / n);
    printf("Average Turnaround Time = %.2f\n", total_turnaround / n);
    printf("(Time Quantum used = %d)\n", TIME_QUANTUM);

    return 0;
}
