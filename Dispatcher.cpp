#define _CRT_SECURE_NO_WARNINGS

#include "Dispatcher.h"
#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <cstring>

// Konstruktor
Dispatcher::Dispatcher(int numWorkers, int numTrucks, int conveyorCapacity, int conveyorMaxWeight, int truckCapacity, int returnTime)
    : numWorkers(numWorkers), numTrucks(numTrucks), activeTruckIndex(0),
    conveyorBelt(conveyorCapacity, conveyorMaxWeight, *this), isRunning(true), signalToTruck(false) {

    // Inicjalizacja semafora
    if (sem_init(&sem, 0, 1) != 0) {
        perror("Blad inicjalizacji semafora");
        throw std::runtime_error("Blad inicjalizacji semafora");
    }

    // Inicjalizacja pamięci dzielonej
    shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Blad otwarcia pamieci dzielonej");
        throw std::runtime_error("Blad otwarcia pamieci dzielonej");
    }

    if (ftruncate(shm_fd, sizeof(int)) == -1) {
        perror("Blad ustawienia rozmiaru pamieci dzielonej");
        throw std::runtime_error("Blad ustawienia rozmiaru pamieci dzielonej");
    }

    shared_data = static_cast<int*>(mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (shared_data == MAP_FAILED) {
        perror("Blad mapowania pamieci dzielonej");
        throw std::runtime_error("Blad mapowania pamieci dzielonej");
    }

    workers.emplace_back(1, 1);
    workers.emplace_back(2, 2);
    workers.emplace_back(3, 3);

    for (int i = 0; i < numTrucks; ++i) {
        trucks.emplace_back(i + 1, truckCapacity, returnTime);
    }
}

// Destruktor
Dispatcher::~Dispatcher() {
    // Niszczenie semafora
    sem_destroy(&sem);

    // Niszczenie pamięci dzielonej
    if (munmap(shared_data, sizeof(int)) == -1) {
        perror("Blad odmapowania pamieci dzielonej");
    }
    if (close(shm_fd) == -1) {
        perror("Blad zamkniecia deskryptora pamieci dzielonej");
    }
    if (shm_unlink("/shared_memory") == -1) {
        perror("Blad usuniecia pamieci dzielonej");
    }
}

void Dispatcher::start() {
    try {
        {
            std::lock_guard<std::mutex> lock(logMutex);
            std::cout << "Dyspozytor: Rozpoczeto symulacje.\n";
        }

        for (auto& worker : workers) {
            worker.startWork(conveyorBelt, isRunning);
        }

        for (auto& truck : trucks) {
            truck.startTransport(conveyorBelt, truckQueueMutex, truckQueueCondition, activeTruckIndex, isRunning);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Blad podczas uruchamiania: " << e.what() << " (" << strerror(errno) << ")\n";
        throw;
    }
}

void Dispatcher::stop() {
    isRunning = false;
    truckQueueCondition.notify_all();
    {
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << "Dyspozytor: Wysylam sygnal stop - zakonczenie pracy.\n";
    }
}

void Dispatcher::waitForCompletion() {
    try {
        for (auto& worker : workers) {
            worker.join();
        }

        for (auto& truck : trucks) {
            truck.join();
        }

        {
            std::lock_guard<std::mutex> lock(logMutex);
            std::cout << "Dyspozytor: Symulacja zakonczona.\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Blad podczas oczekiwania na zakonczenie: " << e.what() << " (" << strerror(errno) << ")\n";
        throw;
    }
}

void Dispatcher::sendSignalToTruck() {
    signalToTruck = true;
    truckQueueCondition.notify_all();
}

bool Dispatcher::getSignalToTruck() const {
    return signalToTruck.load();
}

void Dispatcher::resetSignalToTruck() {
    signalToTruck = false;
}

std::mutex& Dispatcher::getLogMutex() {
    return logMutex;
}

int Dispatcher::getNumTrucks() const {
    return numTrucks;
}