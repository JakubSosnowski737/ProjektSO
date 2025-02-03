#ifndef SYSRESOURCES_H
#define SYSRESOURCES_H

#include <errno.h>
#include <sys/sem.h>
#include <sys/msg.h>

// Globalne identyfikatory zasobów System V
extern int semid;  // Identyfikator zestawu semaforów System V
extern int msgid;  // Identyfikator kolejki komunikatów System V

// Nazwa FIFO (łącza nazwanego)
#define FIFO_NAME "logfifo"

// Definicja struktury komunikatu dla kolejki System V.
struct msgbuf {
    long mtype;
    char mtext[128];
};

#endif // SYSRESOURCES_H
