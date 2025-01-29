#ifndef CONVEYOR_BELT_H
#define CONVEYOR_BELT_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include "Brick.h"

class Dispatcher;

class ConveyorBelt {
private:
    int capacity;
    int maxWeight;
    int currentWeight;
    std::queue<Brick> bricks;
    std::mutex beltMutex;  
    std::condition_variable beltCondition;
    Dispatcher& dispatcher;

public:
    ConveyorBelt(int capacity, int maxWeight, Dispatcher& dispatcher);
    bool addBrick(const Brick& brick);
    bool loadBrick(Brick& brick);
    bool isEmpty(); // Usunięto const

    Dispatcher& getDispatcher();
    std::condition_variable& getBeltCondition();
    std::mutex& getBeltMutex();
};

#endif