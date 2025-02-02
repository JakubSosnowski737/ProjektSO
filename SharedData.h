// SharedData.h
#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <cstring>

struct SharedData {
    int tapeCapacity;
    int tapeWeight;
    int truckCapacity;
    int truckCount;
    int currentTapeWeight;
    int currentTapeCount;
    int semId;
    int truckTime;
    int* truckStatus; // Zmiana z std::vector na wskaźnik
};

const int MSG_TYPE_SIGNAL = 1;

struct Message {
    long mtype;
    int signal;
};

#endif // SHARED_DATA_H