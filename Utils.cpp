// Utils.cpp
#include "Utils.h"
#include <stdexcept>
#include <sys/ipc.h>
#include <sys/shm.h>

void initializeSharedData(SharedData* sharedData, int tapeCapacity, int tapeWeight,
                         int truckCapacity, int truckCount, int truckTime) {
    sharedData->tapeCapacity = tapeCapacity;
    sharedData->tapeWeight = tapeWeight;
    sharedData->truckCapacity = truckCapacity;
    sharedData->truckCount = truckCount;
    sharedData->currentTapeCount = 0;
    sharedData->currentTapeWeight = 0;
    sharedData->semId = createSemaphore();
    sharedData->truckTime = truckTime;
    
    // Inicjalizacja statusów ciężarówek w pamięci współdzielonej
    for(int i = 0; i < truckCount; ++i) {
        sharedData->truckStatus[i] = 1;
    }
}

int createMessageQueue() {
    int msgQueueId = msgget(IPC_PRIVATE, 0666);
    if(msgQueueId == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }
    return msgQueueId;
}

int createSemaphore() {
    int semId = semget(IPC_PRIVATE, 1, 0666);
    if(semId == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }
    if(semctl(semId, 0, SETVAL, 1) == -1) {
        perror("semctl failed");
        exit(EXIT_FAILURE);
    }
    return semId;
}

void validateInput(int tapeCapacity, int tapeWeight, int truckCapacity, int truckCount) {
    if(tapeCapacity <= 0 || tapeWeight <= 0 || truckCapacity <= 0 || truckCount <= 0) {
        throw std::invalid_argument("All values must be positive");
    }
    if(3 * tapeCapacity > tapeWeight) {
        throw std::invalid_argument("Tape weight (M) must be greater than 3*K");
    }
}