// Dispatcher.cpp
#include "Dispatcher.h"
#include <unistd.h>
#include <iostream>
#include <sys/sem.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

Dispatcher::Dispatcher(SharedData* sharedData, int msgQueueId)
    : sharedData(sharedData), msgQueueId(msgQueueId) {}

void Dispatcher::start() {
    int elapsedTime = 0;
    while(true) {
        sleep(5);
        elapsedTime += 5;

        sendSignal(1);

        if(allBricksDelivered() || elapsedTime >= 10) {
            for(int i = 0; i < 3 + sharedData->truckCount; ++i) {
                sendSignal(2);
            }
            break;
        }
    }
    std::cout << "Dispatcher finished work\n";
}

void Dispatcher::sendSignal(int signal) {
    Message msg;
    msg.mtype = MSG_TYPE_SIGNAL;
    msg.signal = signal;

    if(msgsnd(msgQueueId, &msg, sizeof(msg.signal), 0) == -1) {
        perror("msgsnd failed");
    }
}

bool Dispatcher::allBricksDelivered() {
    struct sembuf semOp;
    semOp.sem_num = 0;
    semOp.sem_op = -1;
    semOp.sem_flg = 0;
    
    semop(sharedData->semId, &semOp, 1); // Blokada
    
    bool allDelivered = true;
    for(int i = 0; i < sharedData->truckCount; ++i) {
        if(sharedData->truckStatus[i] != 0) {
            allDelivered = false;
            break;
        }
    }
    
    semOp.sem_op = 1;
    semop(sharedData->semId, &semOp, 1); // Odblokowanie
    
    return allDelivered;
}