#include "Worker.h"
#include <unistd.h>
#include <iostream>
#include <csignal>
#include <sys/sem.h>

// Worker.cpp
Worker::Worker(int id, SharedData* sharedData, int msgQueueId)
    : id(id), sharedData(sharedData), msgQueueId(msgQueueId), running(true) {
    // USUNIĘTO rejestrację handlerów sygnałów
}

void Worker::start() {
    while (running) {
        // Sprawdź, czy są komunikaty w kolejce
        Message msg;
        if (msgrcv(msgQueueId, &msg, sizeof(msg.signal), MSG_TYPE_SIGNAL, IPC_NOWAIT) != -1) {
            handleSignal(msg.signal);
        }

        // Sprawdź, czy pracownik nadal działa
        if (!running) break;

        int brickWeight = id; // P1: 1, P2: 2, P3: 3
        addBrick(brickWeight);
        sleep(1); // Symulacja czasu pracy
    }

    std::cout << "Pracownik P" << id << " zakończył pracę\n";
}

void Worker::addBrick(int weight) {
    struct sembuf semOp;
    semOp.sem_num = 0;
    semOp.sem_op = -1; // Zajmij semafor
    semOp.sem_flg = 0;

    if (semop(sharedData->semId, &semOp, 1) == -1) {
        perror("semop nie powiodło się");
        exit(EXIT_FAILURE);
    }

    // Sprawdź, czy dodanie cegły przekroczy ładowność ciężarówki
    if (sharedData->currentTapeWeight + weight > sharedData->truckCapacity) {
        std::cout << "Pracownik P" << id << " nie może dodać cegły (przekroczona ładowność ciężarówki)\n";
    } else if (sharedData->currentTapeCount < sharedData->tapeCapacity &&
               sharedData->currentTapeWeight + weight <= sharedData->tapeWeight) {
        sharedData->currentTapeCount++;
        sharedData->currentTapeWeight += weight;
        std::cout << "Pracownik P" << id << " dodał cegłę o wadze " << weight << "\n";
    } else {
        std::cout << "Pracownik P" << id << " nie może dodać cegły (taśma pełna)\n";
    }

    semOp.sem_op = 1; // Zwolnij semafor
    if (semop(sharedData->semId, &semOp, 1) == -1) {
        perror("semop nie powiodło się");
        exit(EXIT_FAILURE);
    }
}

void Worker::handleSignal(int signal) {
    if (signal == 1) {
        std::cout << "Pracownik P" << id << " otrzymał sygnał 1\n";
    } else if (signal == 2) {
        std::cout << "Pracownik P" << id << " kończy pracę\n";
        running = false;
    }
}

void Worker::signalHandler(int signum) {
    // Pusta implementacja, ponieważ obsługa sygnałów jest w handleSignal
}