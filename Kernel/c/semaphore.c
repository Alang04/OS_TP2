// Semaforos con nombre. Cada uno tiene un spinlock (xchg) que protege su valor y su
// cola de espera; los procesos que esperan quedan BLOCKED y no consumen CPU.
#include "semaphore.h"
#include "process.h"
#include "lib.h"

typedef struct {
    char name[MAX_SEM_NAME];
    uint8_t used;
    uint8_t lock;
    uint64_t value;
    uint64_t openCount;                 // procesos que lo abrieron y no lo cerraron
    int64_t queue[MAX_PROCESSES];       // pids bloqueados, cola circular FIFO
    uint64_t queueStart;
    uint64_t queueSize;
} Semaphore;

static Semaphore semaphores[MAX_SEMAPHORES];
static uint8_t tableLock = 0;           // protege la creacion y destruccion de semaforos

static uint8_t nameEquals(const char * a, const char * b){
    uint64_t i = 0;
    for(; i < MAX_SEM_NAME - 1 && a[i] != '\0'; i++){
        if(a[i] != b[i]){
            return 0;
        }
    }
    return a[i] == b[i] || i == MAX_SEM_NAME - 1;
}

static void copySemName(char * dest, const char * src){
    uint64_t i = 0;
    for(; i < MAX_SEM_NAME - 1 && src[i] != '\0'; i++){
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

static Semaphore * findSemaphore(const char * name){
    for(int i = 0; i < MAX_SEMAPHORES; i++){
        if(semaphores[i].used && nameEquals(name, semaphores[i].name)){
            return &semaphores[i];
        }
    }
    return 0;
}

static Semaphore * findFreeSemaphore(void){
    for(int i = 0; i < MAX_SEMAPHORES; i++){
        if(!semaphores[i].used){
            return &semaphores[i];
        }
    }
    return 0;
}

static void enqueue(Semaphore * sem, int64_t pid){
    sem->queue[(sem->queueStart + sem->queueSize) % MAX_PROCESSES] = pid;
    sem->queueSize++;
}

static int64_t dequeue(Semaphore * sem){
    int64_t pid = sem->queue[sem->queueStart];
    sem->queueStart = (sem->queueStart + 1) % MAX_PROCESSES;
    sem->queueSize--;
    return pid;
}

int64_t semOpen(const char * name, uint64_t initialValue){
    if(name == 0 || name[0] == '\0'){
        return -1;
    }

    acquireLock(&tableLock);
    Semaphore * sem = findSemaphore(name);
    if(sem == 0){
        sem = findFreeSemaphore();
        if(sem == 0){
            releaseLock(&tableLock);
            return -1;
        }
        copySemName(sem->name, name);
        sem->value = initialValue;
        sem->openCount = 0;
        sem->queueStart = 0;
        sem->queueSize = 0;
        sem->lock = 0;
        sem->used = 1;
    }
    sem->openCount++;
    releaseLock(&tableLock);

    return 0;
}

int64_t semWait(const char * name){
    if(name == 0){
        return -1;
    }

    Semaphore * sem = findSemaphore(name);
    if(sem == 0){
        return -1;
    }

    acquireLock(&sem->lock);
    if(sem->value > 0){
        sem->value--;
        releaseLock(&sem->lock);
        return 0;
    }

    if(sem->queueSize == MAX_PROCESSES){
        releaseLock(&sem->lock);
        return -1;
    }

    // El post que nos despierte nos cede su unidad: al volver no hay que decrementar
    enqueue(sem, getPid());
    releaseLock(&sem->lock);
    blockCurrentProcess(BLOCK_SEM);
    return 0;
}

int64_t semPost(const char * name){
    if(name == 0){
        return -1;
    }

    Semaphore * sem = findSemaphore(name);
    if(sem == 0){
        return -1;
    }

    acquireLock(&sem->lock);
    // Se saltean los que murieron mientras esperaban (kill)
    while(sem->queueSize > 0){
        if(wakeProcess(dequeue(sem), BLOCK_SEM) == 0){
            releaseLock(&sem->lock);
            return 0;
        }
    }
    sem->value++;
    releaseLock(&sem->lock);

    return 0;
}

int64_t semClose(const char * name){
    if(name == 0){
        return -1;
    }

    acquireLock(&tableLock);
    Semaphore * sem = findSemaphore(name);
    if(sem == 0){
        releaseLock(&tableLock);
        return -1;
    }

    sem->openCount--;
    if(sem->openCount == 0){
        sem->used = 0;
    }
    releaseLock(&tableLock);

    return 0;
}
