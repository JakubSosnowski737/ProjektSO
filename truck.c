#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "truck.h"
#include "shared.h"
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <errno.h>
#include "svsem.h"
#include "sysresources.h"

// Funkcje i zmienne statyczne – widoczne tylko w truck.c
static volatile sig_atomic_t force_depart = 0;

static void sigusr1_handler(int signo) {
    force_depart = 1;
}

void run_truck(SharedData *shared, int truck_capacity, int delivery_time, int truck_id) {
    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    while (1) {
        // Zajmujemy miejsce przy taśmie (semafor indeks 2)
        while (1) {
            sem_P(2);
            if (shared->current_truck == 0) {
                shared->current_truck = getpid();
                sem_V(2);
                printf("Ciężarówka %d: Zajęłam miejsce przy taśmie.\n", truck_id);
                break;
            }
            sem_V(2);
            sleep(1);
        }

        // Sprawdzamy stan taśmy i pracowników.
        sem_P(0);
        int remaining = shared->belt.count;
        int local_workers = shared->active_workers;
        int sim_finished = shared->simulation_finished;
        sem_V(0);
        if (remaining == 0 && local_workers == 0) {
            sem_P(2);
            shared->current_truck = 0;
            sem_V(2);
            printf("Ciężarówka %d: Brak cegieł i pracowników. Kończę pracę.\n", truck_id);
            break;
        }

        int cargo = 0;
        // Pętla ładowania cegieł.
        while (cargo < truck_capacity) {
            if (force_depart) {
                printf("Ciężarówka %d: Otrzymałam sygnał wymuszonego odjazdu.\n", truck_id);
                break;
            }
            // Używamy trywait na semaforze bricks_available (indeks 1).
            struct sembuf op = {1, -1, IPC_NOWAIT};
            if (semop(semid, &op, 1) == -1) {
                if (errno == EAGAIN) {
                    sem_P(0);
                    int local_count = shared->belt.count;
                    int local_workers = shared->active_workers;
                    int sim_finished_inner = shared->simulation_finished;
                    sem_V(0);
                    if (sim_finished_inner) break;
                    if (local_count == 0 && local_workers == 0) break;
                    struct timespec req = {0, 100000000};
                    nanosleep(&req, NULL);
                    continue;
                } else {
                    perror("semop trywait bricks_available error");
                    exit(1);
                }
            }
            sem_P(0);
            if (shared->belt.count > 0) {
                Brick b = shared->belt.bricks[shared->belt.head];
                if (cargo + b.weight <= truck_capacity) {
                    cargo += b.weight;
                    shared->belt.head = (shared->belt.head + 1) % shared->belt.K;
                    shared->belt.count--;
                    shared->belt.total_weight -= b.weight;
                    printf("Ciężarówka %d: Załadowano cegłę o wadze %d od P%d. Ładunek: %d/%d.\n",
                           truck_id, b.weight, b.worker_id, cargo, truck_capacity);
                } else {
                    // Jeśli kolejna cegła nie mieści się, natychmiast wychodzimy.
                    sem_V(0);
                    break;
                }
            }
            sem_V(0);
        } // koniec pętli ładowania

        // Zwalniamy miejsce przy taśmie.
        sem_P(2);
        shared->current_truck = 0;
        sem_V(2);
        if (cargo == truck_capacity) {
            printf("Ciężarówka %d: Pełny ładunek (%d). Odjeżdżam.\n", truck_id, cargo);
        } else {
            printf("Ciężarówka %d: Niepełny ładunek (%d). Odjeżdżam (na polecenie lub brak cegieł).\n", truck_id, cargo);
        }
        
        // Wysyłamy komunikat do kolejki komunikatów.
        struct msgbuf msg;
        msg.mtype = truck_id;
        snprintf(msg.mtext, sizeof(msg.mtext), "Ciężarówka %d: Dostarczyła ładunek %d.", truck_id, cargo);
        if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
            perror("msgsnd error in truck");
        }
        
        // Zapisujemy log do FIFO. Otwieramy FIFO do zapisu (tryb O_WRONLY).
        int fifo_fd = open(FIFO_NAME, O_WRONLY);
        if (fifo_fd != -1) {
            char fifo_msg[128];
            snprintf(fifo_msg, sizeof(fifo_msg), "Ciężarówka %d: Rozwożę cegły...\n", truck_id);
            write(fifo_fd, fifo_msg, strlen(fifo_msg));
            close(fifo_fd);
        } else {
            perror("open FIFO for writing in truck");
        }
        
        printf("Ciężarówka %d: Rozwożę cegły...\n", truck_id);
        sleep(delivery_time);
        printf("Ciężarówka %d: Wróciłam do cegielni.\n", truck_id);
        
        sem_P(0);
        remaining = shared->belt.count;
        local_workers = shared->active_workers;
        sim_finished = shared->simulation_finished;
        sem_V(0);
        if (sim_finished) {
            printf("Ciężarówka %d: Symulacja zakończona. Kończę pracę.\n", truck_id);
            break;
        }
        if (remaining == 0 && local_workers == 0) {
            printf("Ciężarówka %d: Brak więcej cegieł. Kończę pracę.\n", truck_id);
            break;
        }
        printf("Ciężarówka %d: Przygotowuję się do kolejnego załadunku.\n", truck_id);
        force_depart = 0;
    }

    exit(0);
}
