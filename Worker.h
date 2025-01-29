#ifndef WORKER_H
#define WORKER_H

#include "ConveyorBelt.h"
#include <atomic>
#include <thread>

class ConveyorBelt;

class Worker {
private:
    int id;
    int brickWeight;
    std::thread workerThread;

public:
    Worker(int id, int brickWeight);
    void startWork(ConveyorBelt& conveyorBelt, std::atomic<bool>& isRunning);
    void join();
};

#endif