#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

// defino el tamaño del heap y su direccion de inicio
//     El kernel está en 0x100000
//     El userland está en 0x400000
#define HEAP_START 0x6000000
#define HEAP_SIZE 0x4000000

#include <stdint.h>

typedef struct {
    uint64_t totalMemory; // bytes totales administradas por el MemoryManager
    uint64_t usedMemory;  // bytes actualmente en uso (ocupados)
    uint64_t freeMemory;  // bytes actualmente libres (disponibles)
    uint64_t blocksUsed; // cantidad de bloques actualmente en uso
} MemoryManagerStats;

// Inicializamos el manager sobre una region de memoria de start hasta start + size
void memoryManagerInit(void * start, uint64_t size);

// Reservamos la cantidad "size" de bytes, devolvemos NULL si no hay suficiente memoria
void * memoryManagerAlloc(uint64_t size);

// Liberamos la memoria previamente reservada en "ptr", si ptr es NULL no hacemos nada
void memoryManagerFree(void * ptr);

// completamos "info" con la informacion del MemoryManager
void memoryManagerGetStats(MemoryManagerStats * info);

#endif