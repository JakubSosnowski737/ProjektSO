#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "dispatcher.h"
#include "shared.h"
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/types.h>
#include "svsem.h"
#include "sysresources.h"

#define DISPATCHER_COLOR "\033[1;35m"
#define RESET_COLOR "\033[0m"

void run_dispatcher(SharedData *shared, pid_t *worker_pids, int num_workers) {
    int interval_signal1 = 8;      // co 10 sekund
    int simulation_duration = 30;   // 60 sekund symulacji
    int elapsed = 0;
    
    printf(DISPATCHER_COLOR "Dyspozytor: Automatyczne wysyłanie sygnałów rozpoczęte.\n" RESET_COLOR);
    fflush(stdout);
    
    while (elapsed < simulation_duration) {
        sleep(interval_signal1);
        elapsed += interval_signal1;
        
        // Używamy semaforu truck_mutex (indeks 2)
        sem_P(2);
        pid_t truck_pid = shared->current_truck;
        sem_V(2);
        if (truck_pid != 0) {
            printf(DISPATCHER_COLOR "Dyspozytor: Automatycznie wysyłam SIGUSR1 do ciężarówki (aktualnie zajęta miejsce przy taśmie).\n" RESET_COLOR);
            fflush(stdout);
            kill(truck_pid, SIGUSR1);
        } else {
            printf(DISPATCHER_COLOR "Dyspozytor: Aktualnie nie ma ciężarówki przy taśmie dla SIGUSR1.\n" RESET_COLOR);
            fflush(stdout);
        }
    }
    
    printf(DISPATCHER_COLOR "Dyspozytor: Czas symulacji upłynął, wysyłam SIGUSR2 do pracowników.\n" RESET_COLOR);
    fflush(stdout);
    for (int i = 0; i < num_workers; i++) {
        kill(worker_pids[i], SIGUSR2);
    }
    
    // Ustawiamy flagę zakończenia symulacji (semafor belt_mutex, indeks 0)
    sem_P(0);
    shared->simulation_finished = 1;
    sem_V(0);
    
    printf(DISPATCHER_COLOR "Dyspozytor: Kończę pracę.\n" RESET_COLOR);
    fflush(stdout);
}
