#ifndef WORKER_H
#define WORKER_H

#include "shared.h"

// Deklaracja funkcji pracownika.
void run_worker(SharedData *shared, int worker_id, int brick_weight);

#endif // WORKER_H
