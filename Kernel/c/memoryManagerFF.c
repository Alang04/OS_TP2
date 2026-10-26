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
 * Inicializa el memory manager: creamos un solo bloque libre que cubre todo el heap
 */
void memoryManagerInit(void * start, uint64_t size){
    
    uint64_t begin = ALIGN_UP((uint64_t)start);

    uint64_t end = ALIGN_DOWN((uint64_t)start + size);

    firstBlock = NULL;
    usedMemory = 0;
    blocksused = 0;

    // chequeamos que no haya errores:
    // * dimension negativa
    // * dimension demasiado chica
    if(end <= begin || (end - begin) <= MIN_SPLIT_SIZE) {
        
        heapStart = heapEnd = 0;
        
        return;
    }


    // ya chequeadas las dimensiones creamos el bloque inicial
    heapStart = begin;
    heapEnd = end;

    firstBlock = (Block*) begin;

    firstBlock->size = (end - begin) - HEADER_SIZE;
    firstBlock->magic = MAGIC_FREE;
    firstBlock->next = NULL;
    firstBlock->prev = NULL;

}

 /**
  * Alloca memoria usando el algoritmo first fit
  * Usamos el primer bloque libre con espacio suficiente
  */
void * memoryManagerAlloc(uint64_t size){

    // chequeamos:
    //  * que se haya inicializado el heap
    //  * que el tamaño pedido sea mayor a 0
    //  * que el tamaño pedido no sea mayor al tamaño del heap

    if(size == 0 || firstBlock == NULL || size > (heapEnd - heapStart)) {
        return NULL;
    }

    size = ALIGN_UP(size);

    // recorro lista de bloques buscando el primer bloque libre con espacio suficiente
    for(Block * b = firstBlock; b != NULL; b = b->next) {

        // bloque ocupado o demasiado chico
        if(b->magic != MAGIC_FREE || b->size < size) {
            continue;
        }

        // bloque libre y suficientemente grande para partirlo
        if(b->size >= size + MIN_SPLIT_SIZE) {

            
            // parto entre b (bloque a ocupar) y rest (bloque libre que queda)
            Block * rest = (Block *)((uint64_t)(b + 1) + size);
            rest->size = b->size - size - HEADER_SIZE;
            rest->magic = MAGIC_FREE;
            rest->prev = b;
            rest->next = b->next;

            if(b->next != NULL) {
                b->next->prev = rest;
            }
            
            b->next = rest;
            b->size = size;

            // ahora ya se que b tiene el tamaño exacto que necesito
        }

        // ocupo el bloque b
        b->magic = MAGIC_USED;
        usedMemory += b->size + HEADER_SIZE;
        blocksused++;

        return (void *)(b + 1); // devuelvo la dirección del bloque usable (después del header)

    }

    // no habia espacio disponible
    return NULL;
}

/**
 * Libera memoria reservada
 */
void memoryManagerFree(void * ptr){
    
}