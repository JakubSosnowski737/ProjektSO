#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "worker.h"
#include "shared.h"
#include <sys/sem.h>
#include <fcntl.h>
#include <time.h>
#include "svsem.h"
#include "sysresources.h"

volatile sig_atomic_t stop_work = 0;

void sigusr2_handler(int signo) {
    stop_work = 1;
}

void run_worker(SharedData *shared, int worker_id, int brick_weight) {
    struct sigaction sa;
    sa.sa_handler = sigusr2_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR2, &sa, NULL);

    srand(time(NULL) ^ (worker_id * 100));

    int brick_seq = 0;
    while (!stop_work) {
        sem_P(0); // lock belt_mutex (indeks 0)
        if (shared->belt.count < shared->belt.K &&
            (shared->belt.total_weight + brick_weight) <= shared->belt.M) {
            Brick b;
            b.weight = brick_weight;
            b.worker_id = worker_id;
            b.seq = brick_seq++;
            shared->belt.bricks[shared->belt.tail] = b;
            shared->belt.tail = (shared->belt.tail + 1) % shared->belt.K;
            shared->belt.count++;
            shared->belt.total_weight += brick_weight;
            printf("Pracownik P%d: Dodano cegłę o wadze %d. (Cegieł: %d, masa: %d)\n",
                   worker_id, brick_weight, shared->belt.count, shared->belt.total_weight);
            sem_V(1); // signal bricks_available (indeks 1)
            sem_V(0); // unlock belt_mutex
            struct timespec delay;
            delay.tv_sec = 0;
            delay.tv_nsec = (50 + rand() % 101) * 1000000L; // 50-150 ms
            nanosleep(&delay, NULL);
        } else {
            sem_V(0);
            struct timespec req = {0, 100000000}; // 100 ms
            nanosleep(&req, NULL);
        }
    }
    printf("Pracownik P%d: Otrzymałem sygnał zakończenia. Kończę pracę.\n", worker_id);
    
    sem_P(0);
    shared->active_workers--;
    sem_V(0);

    exit(0);
}
