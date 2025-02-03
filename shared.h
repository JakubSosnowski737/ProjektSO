#ifndef SHARED_H
#define SHARED_H

#include <semaphore.h>
#include <sys/types.h>

// Maksymalne wartości dla danych wejściowych:
#define MAX_K 100   // Maksymalna liczba cegieł na taśmie
#define MAX_M 100   // Maksymalna masa cegieł na taśmie
#define MAX_C 300   // Maksymalna ładowność ciężarówki
#define MAX_N 10    // Maksymalna liczba ciężarówek
#define MAX_T 100   // Maksymalny czas powrotu ciężarówki (w sekundach)

// Struktura reprezentująca pojedynczą cegłę
typedef struct {
    int weight;      // waga cegły
    int worker_id;   // numer pracownika (P1, P2, P3) który ją wyprodukował
    int seq;         // opcjonalny numer sekwencyjny
} Brick;

// Struktura reprezentująca taśmę transportową
typedef struct {
    int K;           // maksymalna liczba cegieł, jakie mogą być jednocześnie na taśmie
    int M;           // maksymalna łączna masa cegieł na taśmie
    int count;       // bieżąca liczba cegieł
    int total_weight;// bieżąca łączna masa cegieł
    int head;        // indeks pierwszej cegły (do zdejmowania – FIFO)
    int tail;        // indeks, pod który trafia nowa cegła
    Brick bricks[MAX_K]; // tablica cegieł (zakładamy, że K <= MAX_K)
} Belt;

// Struktura danych współdzielonych
typedef struct {
    Belt belt;
    int simulation_finished; // ustawiana, gdy dyspozytor poda polecenie zakończenia (SIGUSR2)
    int active_workers;      // liczba jeszcze pracujących pracowników
    pid_t current_truck;     // PID ciężarówki aktualnie zajmującej miejsce przy taśmie (0, gdy nikt)
} SharedData;

#endif // SHARED_H
