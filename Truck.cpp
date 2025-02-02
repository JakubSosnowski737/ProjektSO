#include "Truck.h"
#include <unistd.h>
#include <iostream>
#include <csignal>
#include <sys/sem.h>

Truck::Truck(int id, SharedData* sharedData, int msgQueueId)
    : id(id), sharedData(sharedData), msgQueueId(msgQueueId), running(true) {
    // USUNIĘTO rejestrację handlerów sygnałów
}
void Truck::start() {
    while (running) {
        // Sprawdź, czy są komunikaty w kolejce
        Message msg;
        if (msgrcv(msgQueueId, &msg, sizeof(msg.signal), MSG_TYPE_SIGNAL, IPC_NOWAIT) != -1) {
            handleSignal(msg.signal);
        }

        // Sprawdź, czy ciężarówka nadal działa
        if (!running) break;

        loadBricks();
        sleep(sharedData->truckTime); // Symulacja czasu jazdy
    }

    std::cout << "Ciężarówka T" << id << " zakończyła pracę\n";
}

void Truck::loadBricks() {
    struct sembuf semOp;
    semOp.sem_num = 0;
    semOp.sem_op = -1; // Zajmij semafor
    semOp.sem_flg = 0;

    if (semop(sharedData->semId, &semOp, 1) == -1) {
        perror("semop nie powiodło się");
        exit(EXIT_FAILURE);
    }

    if (sharedData->currentTapeCount > 0) {
        // Sprawdź, czy dodanie kolejnej cegły przekroczy ładowność ciężarówki
        if (sharedData->currentTapeWeight + sharedData->currentTapeCount > sharedData->truckCapacity) {
            std::cout << "Ciężarówka T" << id << " odjeżdża z niepełnym ładunkiem (" << sharedData->currentTapeWeight << "/" << sharedData->truckCapacity << ")\n";
            sharedData->currentTapeCount = 0;
            sharedData->currentTapeWeight = 0;
        } else {
            std::cout << "Ciężarówka T" << id << " załadowała cegły (" << sharedData->currentTapeWeight << "/" << sharedData->truckCapacity << ")\n";
            sharedData->currentTapeCount = 0;
            sharedData->currentTapeWeight = 0;
        }
    }

    semOp.sem_op = 1; // Zwolnij semafor
    if (semop(sharedData->semId, &semOp, 1) == -1) {
        perror("semop nie powiodło się");
        exit(EXIT_FAILURE);
    }
}

void Truck::handleSignal(int signal) {
    if (signal == 1) {
        std::cout << "Ciężarówka T" << id << " odjeżdża z niepełnym ładunkiem\n";
        loadBricks(); // Załaduj dostępne cegły
        running = false; // Zakończ pracę ciężarówki
    } else if (signal == 2) {
        std::cout << "Ciężarówka T" << id << " kończy pracę\n";
        running = false;
    }
}

void Truck::signalHandler(int signum) {
    // Pusta implementacja, ponieważ obsługa sygnałów jest w handleSignal
}