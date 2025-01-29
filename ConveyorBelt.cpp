#include "ConveyorBelt.h"

ConveyorBelt::ConveyorBelt(int capacity, int maxWeight, Dispatcher& dispatcher)
    : capacity(capacity), maxWeight(maxWeight), currentWeight(0), dispatcher(dispatcher) {}

bool ConveyorBelt::addBrick(const Brick& brick) {
    std::lock_guard<std::mutex> lock(beltMutex);
    if (bricks.size() < capacity && currentWeight + brick.getWeight() <= maxWeight) {
        bricks.push(brick);
        currentWeight += brick.getWeight();
        return true;
    }
    return false;
}

bool ConveyorBelt::loadBrick(Brick& brick) {
    std::lock_guard<std::mutex> lock(beltMutex);
    if (!bricks.empty()) {
        brick = bricks.front();
        bricks.pop();
        currentWeight -= brick.getWeight();
        return true;
    }
    return false;
}

bool ConveyorBelt::isEmpty() const {
    return bricks.empty();
}

Dispatcher& ConveyorBelt::getDispatcher() {
    return dispatcher;
}

