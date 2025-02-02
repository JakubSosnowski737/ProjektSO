// Utils.h
#ifndef UTILS_H
#define UTILS_H

#include "SharedData.h"

void initializeSharedData(SharedData* sharedData, int tapeCapacity, int tapeWeight, 
                         int truckCapacity, int truckCount, int truckTime);
int createMessageQueue();
int createSemaphore();
void validateInput(int tapeCapacity, int tapeWeight, int truckCapacity, int truckCount);

#endif // UTILS_H