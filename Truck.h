#ifndef TRUCK_H
#define TRUCK_H

#include "ConveyorBelt.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ConveyorBelt;

class Truck {
private:
    int id;
    int maxWeight;
    int currentLoad;
    int returnTime;
    std::thread truckThread;

public:
    Truck(int id, int maxWeight, int returnTime);
    void startTransport(ConveyorBelt& conveyorBelt, std::mutex& queueMutex,
        std::condition_variable& queueCondition, int& activeTruckIndex,
        std::atomic<bool>& isRunning);
    void transportToDestination(ConveyorBelt& conveyorBelt, std::mutex& queueMutex,
        std::condition_variable& queueCondition, int& activeTruckIndex);
    bool addBrick(const Brick& brick);
    void join();
};

#endif