#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "Worker.h"
#include "Truck.h"
#include "ConveyorBelt.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <semaphore.h> 
#include <sys/mman.h> 
#include <fcntl.h>
#include <unistd.h>

class Worker;
class Truck;
class ConveyorBelt;

class Dispatcher {
private:
    std::atomic<bool> isRunning;
    int numWorkers;
    int numTrucks;
    ConveyorBelt conveyorBelt;
    std::vector<Worker> workers;
    std::vector<Truck> trucks;

    std::mutex truckQueueMutex;
    std::condition_variable truckQueueCondition;
    int activeTruckIndex;

    std::mutex logMutex;

    std::atomic<bool> signalToTruck;

    sem_t sem; // Zmienna semafora
    int shm_fd; // Deskryptor pamięci dzielonej
    int* shared_data; // Wskaźnik do pamięci dzielonej

public:
    Dispatcher(int numWorkers, int numTrucks, int conveyorCapacity, int conveyorMaxWeight, int truckCapacity, int returnTime);
    ~Dispatcher();

    void start();
    void stop();
    void waitForCompletion();
    void sendSignalToTruck();

    std::mutex& getLogMutex();
    int getNumTrucks() const;
    bool getSignalToTruck() const;
    void resetSignalToTruck();
};

#endif