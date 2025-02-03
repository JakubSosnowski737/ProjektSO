#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "shared.h"

// Deklaracja funkcji dyspozytora.
void run_dispatcher(SharedData *shared, pid_t *worker_pids, int num_workers);

#endif // DISPATCHER_H
