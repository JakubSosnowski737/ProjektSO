#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "SharedData.h"

class Dispatcher {
public:
    Dispatcher(SharedData* sharedData, int msgQueueId);
    void start();

private:
    SharedData* sharedData;
    int msgQueueId;
    void sendSignal(int signal);
    bool allBricksDelivered(); // Dodana deklaracja metody
};

#endif // DISPATCHER_H