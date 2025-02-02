// main.cpp
#include "SharedData.h"
#include "Worker.h"
#include "Truck.h"
#include "Dispatcher.h"
#include "Utils.h"
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>

int main() {
    try {
        int tapeCapacity, tapeWeight, truckCapacity, truckCount, truckTime;
        
        std::cout << "Enter tape capacity (K): ";
        std::cin >> tapeCapacity;
        std::cout << "Enter max tape weight (M): ";
        std::cin >> tapeWeight;
        std::cout << "Enter truck capacity (C): ";
        std::cin >> truckCapacity;
        std::cout << "Enter number of trucks (N): ";
        std::cin >> truckCount;
        std::cout << "Enter truck return time (Ti): ";
        std::cin >> truckTime;

        validateInput(tapeCapacity, tapeWeight, truckCapacity, truckCount);

        // Alokacja pamięci współdzielonej
        size_t sharedSize = sizeof(SharedData) + truckCount * sizeof(int);
        int shmid = shmget(IPC_PRIVATE, sharedSize, 0666);
        SharedData* sharedData = (SharedData*)shmat(shmid, NULL, 0);
        
        // Inicjalizacja wskaźnika truckStatus
        sharedData->truckStatus = (int*)((char*)sharedData + sizeof(SharedData));
        
        initializeSharedData(sharedData, tapeCapacity, tapeWeight, 
                           truckCapacity, truckCount, truckTime);

        int msgQueueId = createMessageQueue();

        // Procesy pracowników
        for(int i = 1; i <= 3; ++i) {
            if(fork() == 0) {
                Worker worker(i, sharedData, msgQueueId);
                worker.start();
                exit(0);
            }
        }

        // Procesy ciężarówek
        for(int i = 1; i <= truckCount; ++i) {
            if(fork() == 0) {
                Truck truck(i, sharedData, msgQueueId);
                truck.start();
                exit(0);
            }
        }

        // Proces dyspozytora
        if(fork() == 0) {
            Dispatcher dispatcher(sharedData, msgQueueId);
            dispatcher.start();
            exit(0);
        }

        // Oczekiwanie na zakończenie procesów
        for(int i = 0; i < 3 + truckCount + 1; ++i) {
            wait(NULL);
        }

        // Sprzątanie zasobów
        shmdt(sharedData);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(sharedData->semId, 0, IPC_RMID);
        msgctl(msgQueueId, IPC_RMID, NULL);

    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}