#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <limits>
#include <thread>
#include <cerrno>
#include <cstring>
#include "Dispatcher.h"

int getUserInput(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.fail() || value < min || value > max) {
            std::cerr << "Niepoprawna wartosc. Podaj liczbe w zakresie " << min << "-" << max << ".\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else {
            break;
        }
    }
    return value;
}

void sendSignalToTruck(Dispatcher& dispatcher) {
    std::cout << "Dyspozytor: Wysylam sygnal 1 - ciezarowka moze odjechac z niepelnym ladunkiem.\n";
    dispatcher.sendSignalToTruck();
}

int main() {
    int conveyorCapacity = getUserInput("Podaj pojemnosc tasmy: ", 1, 20);
    int conveyorMaxWeight = getUserInput("Podaj maksymalna wage tasmy: ", 1, 100);
    int truckCapacity = getUserInput("Podaj ladownosc ciezarowki: ", 1, 100);
    int numTrucks = getUserInput("Podaj liczbe ciezarowek: ", 1, 10);
    int returnTime = getUserInput("Podaj czas powrotu ciezarowki (w sekundach): ", 1, 60);

    try {
        Dispatcher dispatcher(3, numTrucks, conveyorCapacity, conveyorMaxWeight, truckCapacity, returnTime);
        dispatcher.start();

        std::this_thread::sleep_for(std::chrono::seconds(5));
        sendSignalToTruck(dispatcher);

        std::this_thread::sleep_for(std::chrono::seconds(10));

        dispatcher.stop();
        dispatcher.waitForCompletion();
    }
    catch (const std::exception& e) {
        std::cerr << "Wystapil blad: " << e.what() << " (" << strerror(errno) << ")\n";
        return EXIT_FAILURE;
    }

    return 0;
}