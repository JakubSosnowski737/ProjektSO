#include "Truck.h"
#include "Dispatcher.h"
#include <iostream>
#include <chrono>

Truck::Truck(int id, int maxWeight, int returnTime) : id(id), maxWeight(maxWeight), currentLoad(0), returnTime(returnTime) {}

bool Truck::addBrick(const Brick& brick) {
    if (currentLoad + brick.getWeight() <= maxWeight) {
        currentLoad += brick.getWeight();
        return true;
    }
    return false;
}

void Truck::transportToDestination(ConveyorBelt& conveyorBelt, std::mutex& queueMutex,
    std::condition_variable& queueCondition, int& activeTruckIndex) {
        {
            std::lock_guard<std::mutex> lock(conveyorBelt.getDispatcher().getLogMutex());
            std::cout << "Ciezarowka " << id << " transportuje cegly o wadze " << currentLoad << ".\n";
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            activeTruckIndex = (activeTruckIndex + 1) % conveyorBelt.getDispatcher().getNumTrucks();
            queueCondition.notify_all();
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
        {
            std::lock_guard<std::mutex> lock(conveyorBelt.getDispatcher().getLogMutex());
            std::cout << "Ciezarowka " << id << " wraca do cegielni.\n";
        }
        currentLoad = 0;
        std::this_thread::sleep_for(std::chrono::seconds(returnTime));
}

void Truck::startTransport(ConveyorBelt& conveyorBelt, std::mutex& queueMutex,
    std::condition_variable& queueCondition, int& activeTruckIndex,
    std::atomic<bool>& isRunning) {
    truckThread = std::thread([this, &conveyorBelt, &queueMutex, &queueCondition, &activeTruckIndex, &isRunning]() {
        while (isRunning || !conveyorBelt.isEmpty()) {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCondition.wait(lock, [&]() { return activeTruckIndex == id - 1 || !isRunning || conveyorBelt.getDispatcher().getSignalToTruck(); });

                if (!isRunning && conveyorBelt.isEmpty()) break;
            }

            Brick brick(0);
            bool canLoadMore = true;
            while (currentLoad < maxWeight && canLoadMore && isRunning && !conveyorBelt.getDispatcher().getSignalToTruck()) {
                if (conveyorBelt.loadBrick(brick)) {
                    if (!addBrick(brick)) {
                        canLoadMore = false;
                    }
                    else {
                        {
                            std::lock_guard<std::mutex> logLock(conveyorBelt.getDispatcher().getLogMutex());
                            std::cout << "Ciezarowka " << id << " zaladowala cegle o wadze " << brick.getWeight()
                                << ". Aktualne zaladowanie: " << currentLoad << " / " << maxWeight << ".\n";
                        }
                    }
                }
                else {
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        queueCondition.wait_for(lock, std::chrono::milliseconds(100), [&]() { return !conveyorBelt.isEmpty() || !isRunning || conveyorBelt.getDispatcher().getSignalToTruck(); });
                    }
                }
            }

            if (currentLoad > 0 || conveyorBelt.getDispatcher().getSignalToTruck()) {
                transportToDestination(conveyorBelt, queueMutex, queueCondition, activeTruckIndex);
                conveyorBelt.getDispatcher().resetSignalToTruck(); 
            }
            else {
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    activeTruckIndex = (activeTruckIndex + 1) % conveyorBelt.getDispatcher().getNumTrucks();
                    queueCondition.notify_all();
                }
            }
        }
        });
}

void Truck::join() {
    if (truckThread.joinable()) {
        truckThread.join();
    }
}
