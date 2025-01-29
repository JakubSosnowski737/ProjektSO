#ifndef CONVEYOR_BELT_H
#define CONVEYOR_BELT_H

#include <queue>
#include <mutex>
#include "Brick.h"

class Dispatcher;

class ConveyorBelt {
private:
    int capacity;
    int maxWeight;
    int currentWeight;
    std::queue<Brick> bricks;
    std::mutex beltMutex;
    Dispatcher& dispatcher;

public:
    ConveyorBelt(int capacity, int maxWeight, Dispatcher& dispatcher);
    bool addBrick(const Brick& brick);
    bool loadBrick(Brick& brick);
    bool isEmpty() const;

    Dispatcher& getDispatcher();
};

#endif