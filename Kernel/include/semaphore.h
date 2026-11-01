#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>

#define MAX_SEMAPHORES 64
#define MAX_SEM_NAME 32

// Los semaforos se identifican por nombre: procesos no relacionados los comparten
// acordando el nombre de antemano. Todas devuelven 0 si salio bien y -1 si fallo.

// Abre el semaforo 'name'; si no existe lo crea con 'initialValue'
int64_t semOpen(const char * name, uint64_t initialValue);

// Decrementa el valor; si es 0 bloquea al proceso (sin busy waiting) hasta un post
int64_t semWait(const char * name);

// Despierta al primero que espera (FIFO) o, si no hay nadie, incrementa el valor
int64_t semPost(const char * name);

// Cierra el semaforo; se destruye cuando lo cierran todos los que lo abrieron
int64_t semClose(const char * name);

#endif
