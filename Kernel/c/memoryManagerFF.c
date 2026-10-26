/**
 * 
 * Vamos a implementar un memory manager con el algoritmo tipo "first fit"
 */

#include <stddef.h>
#include "memoryManager.h"

#define ALIGNMENT 16
// macros para alinear
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(uint64_t)(ALIGNMENT - 1))
#define ALIGN_DOWN(x) ((x) & ~(uint64_t)(ALIGNMENT - 1))


// usamos magic para informar si un bloque está libre o ocupado
#define MAGIC_FREE 0xF3EEF3EEF3EEF3EEULL
#define MAGIC_USED 0xA110CA7EA110CA7EULL

#define HEADER_SIZE sizeof(Block)

 // tamaño mínimo de un bloque para poder dividirlo en dos
#define MIN_SPLIT_SIZE (HEADER_SIZE + ALIGNMENT)

// header que van a tener todos los bloques de memoria
// va a ser una lista ordenada que cubra todo el heap sin huecos
// dos bloques vecinos en la lista van a estar uno al lado del otro en memoria
typedef struct Block {

    uint64_t size; // bytes del bloque que son usables (no contamos el header)
    uint64_t magic; // es o MAGIC_FREE o MAGIC_USED
    struct Block * next; // siguiente bloque en la lista
    struct Block * prev; // bloque anterior en la lista

} Block;

/*
    Variables globales para el manejo de memoria:
*/

static Block * firstBlock = NULL;

static uint64_t heapStart = 0;
static uint64_t heapEnd = 0;

// bytes ocupados contando headers
static uint64_t usedMemory = 0;
static uint64_t blocksused = 0;



/**
 * Inicializa el memory manager
 */
void memoryManagerInit(void * ptr){

}

 /**
  * Alloca memoria usando el algoritmo first fit
  * Usamos el primer bloque libre con espacio suficiente
  */
void * memoryManagerAlloc(uint64_t size){

    return NULL;
}

/**
 * Libera memoria reservada
 */
void memoryManagerFree(void * ptr){
    
}