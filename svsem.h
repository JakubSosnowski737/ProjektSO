#ifndef SVSEM_H
#define SVSEM_H

#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include "sysresources.h"

static inline void sem_P(int sem_num) {
    struct sembuf op = {sem_num, -1, 0};
    if (semop(semid, &op, 1) == -1) {
        perror("semop P error");
        exit(1);
    }
}

static inline void sem_V(int sem_num) {
    struct sembuf op = {sem_num, 1, 0};
    if (semop(semid, &op, 1) == -1) {
        perror("semop V error");
        exit(1);
    }
}

#endif // SVSEM_H
