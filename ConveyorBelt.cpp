#include "ConveyorBelt.h"

ConveyorBelt::ConveyorBelt(int capacity, int maxWeight, Dispatcher& dispatcher)
    : capacity(capacity), maxWeight(maxWeight), currentWeight(0), dispatcher(dispatcher) {}

bool ConveyorBelt::addBrick(const Brick& brick) {
    std::lock_guard<std::mutex> lock(beltMutex);
    if (bricks.size() < capacity && currentWeight + brick.getWeight() <= maxWeight) {
        bricks.push(brick);
        currentWeight += brick.getWeight();
        beltCondition.notify_all();  // Notify trucks that a new brick is available
        return true;
    }
    return false;
}

bool ConveyorBelt::loadBrick(Brick& brick) {
    std::unique_lock<std::mutex> lock(beltMutex);
    beltCondition.wait(lock, [this]() { return !bricks.empty(); }); // Wait until there is a brick available
    if (!bricks.empty()) {
        brick = bricks.front();
        bricks.pop();
        currentWeight -= brick.getWeight();
        return true;
    }
    return false;
}

bool ConveyorBelt::isEmpty() {
    std::lock_guard<std::mutex> lock(beltMutex);
    return bricks.empty();
}

Dispatcher& ConveyorBelt::getDispatcher() {
    return dispatcher;
}

std::condition_variable& ConveyorBelt::getBeltCondition() {
    return beltCondition;
}

std::mutex& ConveyorBelt::getBeltMutex() {
    return beltMutex;
}