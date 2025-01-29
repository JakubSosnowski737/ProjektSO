#include "Worker.h"
#include "Dispatcher.h"

#include <iostream>
#include <chrono>

Worker::Worker(int id, int brickWeight) : id(id), brickWeight(brickWeight) {}

void Worker::startWork(ConveyorBelt& conveyorBelt, std::atomic<bool>& isRunning) {
    workerThread = std::thread([this, &conveyorBelt, &isRunning]() {
        while (isRunning) {
            if (conveyorBelt.addBrick(Brick(brickWeight))) {
                std::lock_guard<std::mutex> lock(conveyorBelt.getDispatcher().getLogMutex());
                std::cout << "Pracownik " << id << " dodal cegle o wadze " << brickWeight << " na tasme.\n";
            } else {
                std::lock_guard<std::mutex> lock(conveyorBelt.getDispatcher().getLogMutex());
                std::cout << "Pracownik " << id << " czeka, bo tasma jest pelna.\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        {
            std::lock_guard<std::mutex> lock(conveyorBelt.getDispatcher().getLogMutex());
            std::cout << "Pracownik " << id << " zakonczyl prace.\n";
        }
    });
}

void Worker::join() {
    if (workerThread.joinable()) {
        workerThread.join();
    }
}