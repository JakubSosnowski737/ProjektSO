#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include "shared.h"
#include "dispatcher.h"
#include "worker.h"
#include "truck.h"
#include "sysresources.h"

// Globalne identyfikatory dla zasobów System V:
int semid;  // Globalny identyfikator zestawu semaforów System V
int msgid;  // Globalny identyfikator kolejki komunikatów System V

int main(int argc, char *argv[]) {
    if(argc < 6) {
        fprintf(stderr, "Usage: %s K M C N T\n", argv[0]);
        fprintf(stderr, "K - maksymalna liczba cegieł na taśmie\n");
        fprintf(stderr, "M - maksymalna masa cegieł na taśmie\n");
        fprintf(stderr, "C - ładowność ciężarówki\n");
        fprintf(stderr, "N - liczba ciężarówek\n");
        fprintf(stderr, "T - czas powrotu ciężarówki (w sekundach)\n");
        exit(1);
    }
    int K = atoi(argv[1]);
    int M = atoi(argv[2]);
    int C = atoi(argv[3]);
    int num_trucks = atoi(argv[4]);
    int T = atoi(argv[5]);

    // Sprawdzamy, czy wszystkie argumenty są dodatnie.
    if(K <= 0 || M <= 0 || C <= 0 || num_trucks <= 0 || T <= 0) {
        fprintf(stderr, "Wszystkie argumenty muszą być dodatnimi liczbami.\n");
        exit(1);
    }
    // Sprawdzamy maksymalne wartości.
    if(K > MAX_K) {
        fprintf(stderr, "K nie może być większe niż %d\n", MAX_K);
        exit(1);
    }
    if(M > MAX_M) {
        fprintf(stderr, "M nie może być większe niż %d\n", MAX_M);
        exit(1);
    }
    if(C > MAX_C) {
        fprintf(stderr, "C nie może być większe niż %d\n", MAX_C);
        exit(1);
    }
    if(num_trucks > MAX_N) {
        fprintf(stderr, "N nie może być większe niż %d\n", MAX_N);
        exit(1);
    }
    if(T > MAX_T) {
        fprintf(stderr, "T nie może być większe niż %d\n", MAX_T);
        exit(1);
    }

    // Tworzymy FIFO (nazywane potok) – zostanie utworzone w katalogu, z którego uruchamiasz program.
    if(mkfifo(FIFO_NAME, 0666) == -1) {
        if(errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }


    int fifo_fd_main = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
    if(fifo_fd_main == -1) {
        perror("open FIFO for reading");
        exit(1);
    }

    // Generujemy klucze przy użyciu ftok.
    key_t key_shm = ftok(".", 'S');
    if(key_shm == -1) { perror("ftok shm"); exit(1); }
    key_t key_sem = ftok(".", 'E');
    if(key_sem == -1) { perror("ftok sem"); exit(1); }
    key_t key_msg = ftok(".", 'M');
    if(key_msg == -1) { perror("ftok msg"); exit(1); }

    // Tworzymy segment pamięci współdzielonej System V.
    int shmid = shmget(key_shm, sizeof(SharedData), IPC_CREAT | 0666);
    if(shmid < 0) { perror("shmget"); exit(1); }
    SharedData *shared = shmat(shmid, NULL, 0);
    if(shared == (void*) -1) { perror("shmat"); exit(1); }

    // Inicjalizujemy dane w pamięci współdzielonej.
    shared->belt.K = K;
    shared->belt.M = M;
    shared->belt.count = 0;
    shared->belt.total_weight = 0;
    shared->belt.head = 0;
    shared->belt.tail = 0;
    shared->simulation_finished = 0;
    shared->active_workers = 3;
    shared->current_truck = 0;

    // Tworzymy zestaw semaforów System V (3 semafory: 
    // 0 - belt_mutex, 1 - bricks_available, 2 - truck_mutex).
    semid = semget(key_sem, 3, IPC_CREAT | 0666);
    if(semid < 0) { perror("semget"); exit(1); }
    if(semctl(semid, 0, SETVAL, 1) == -1) { perror("semctl belt_mutex"); exit(1); }
    if(semctl(semid, 1, SETVAL, 0) == -1) { perror("semctl bricks_available"); exit(1); }
    if(semctl(semid, 2, SETVAL, 1) == -1) { perror("semctl truck_mutex"); exit(1); }

    // Tworzymy kolejkę komunikatów System V.
    msgid = msgget(key_msg, IPC_CREAT | 0666);
    if(msgid < 0) { perror("msgget"); exit(1); }

    // Tworzymy procesy pracowników.
    pid_t worker_pids[3];
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if(pid < 0) { perror("fork worker"); exit(1); }
        if(pid == 0) {
            int brick_weight = i + 1;
            run_worker(shared, i+1, brick_weight);
            exit(0);
        } else {
            worker_pids[i] = pid;
        }
    }

    // Tworzymy procesy ciężarówek.
    pid_t truck_pids[num_trucks];
    for (int i = 0; i < num_trucks; i++) {
        pid_t pid = fork();
        if(pid < 0) { perror("fork truck"); exit(1); }
        if(pid == 0) {
            run_truck(shared, C, T, i+1);
            exit(0);
        } else {
            truck_pids[i] = pid;
        }
    }

    // Tworzymy proces dyspozytora.
    pid_t dispatcher_pid = fork();
    if(dispatcher_pid < 0) { perror("fork dispatcher"); exit(1); }
    if(dispatcher_pid == 0) {
        run_dispatcher(shared, worker_pids, 3);
        exit(0);
    }

    // Czekamy na zakończenie procesów potomnych.
    waitpid(dispatcher_pid, NULL, 0);
    for (int i = 0; i < 3; i++) {
        waitpid(worker_pids[i], NULL, 0);
    }
    for (int i = 0; i < num_trucks; i++) {
        waitpid(truck_pids[i], NULL, 0);
    }

    // Po zakończeniu symulacji odczytujemy komunikaty z kolejki.
    struct msgbuf msg;
    printf("Odebrane komunikaty z kolejki:\n");
    while(msgrcv(msgid, &msg, sizeof(msg.mtext), 0, IPC_NOWAIT) != -1) {
        printf("%s\n", msg.mtext);
    }
    if(errno != ENOMSG) {
        perror("msgrcv");
    }

    // Odczytujemy komunikaty z FIFO przy użyciu uchwytu fifo_fd_main.
    printf("Odebrane komunikaty z FIFO:\n");
    char fifo_buf[256];
    ssize_t n;
    while((n = read(fifo_fd_main, fifo_buf, sizeof(fifo_buf)-1)) > 0) {
        fifo_buf[n] = '\0';
        printf("%s", fifo_buf);
    }
    close(fifo_fd_main);

    // Sprzątamy: usuwamy zestaw semaforów, kolejkę komunikatów, segment pamięci oraz FIFO.
    if(semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID");
    }
    if(msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl IPC_RMID");
    }
    if(shmdt(shared) == -1) {
        perror("shmdt");
    }
    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
    }
    unlink(FIFO_NAME);

    printf("Symulacja zakończona.\n");
    return 0;
}
