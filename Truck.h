#ifndef TRUCK_H
#define TRUCK_H

#include "SharedData.h"

class Truck {
public:
    Truck(int id, SharedData* sharedData, int msgQueueId);
    void start();

private:
    int id;
    SharedData* sharedData;
    int msgQueueId;
    bool running; // Dodano zmienną running
    void loadBricks();
    void handleSignal(int signal);
    static void signalHandler(int signum);
};

#endif // TRUCK_H