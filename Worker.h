#ifndef WORKER_H
#define WORKER_H

#include "SharedData.h"

class Worker {
public:
    Worker(int id, SharedData* sharedData, int msgQueueId);
    void start();

private:
    int id;
    SharedData* sharedData;
    int msgQueueId;
    bool running; // Dodano zmienną running
    void addBrick(int weight);
    void handleSignal(int signal);
    static void signalHandler(int signum);
};

#endif // WORKER_H