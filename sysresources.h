#ifndef SYSRESOURCES_H
#define SYSRESOURCES_H

#include <errno.h>
#include <sys/sem.h>
#include <sys/msg.h>

extern int semid;  // Globalny identyfikator zestawu semaforów System V
extern int msgid;  // Globalny identyfikator kolejki komunikatów System V

#define FIFO_NAME "logfifo"

// Definicja struktury komunikatu dla kolejki System V.
struct msgbuf {
    long mtype;
    char mtext[128];
};

#endif // SYSRESOURCES_H
