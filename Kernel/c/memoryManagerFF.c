/**
 * 
 * Vamos a implementar un memory manager con el algoritmo tipo "first fit"
 */

#include <stddef.h>
#include "memoryManager.h"

#define ALIGNMENT 16
// macros para alinear
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(uint_64t)(ALIGNMENT - 1))
#define ALIGN_DOWN(x) ((x) & ~(uint_64t)(ALIGNMENT - 1))


// usamos magic para informar si un bloque está libre o ocupado
#define MAGIC_FREE 0xF3EEF3EEF3EEF3EEULL
#define MAGIC_USED 0xA110CA7EA110CA7EULL

// header que van a tener todos los bloques de memoria
// va a ser una lista ordenada que cubra todo el heap sin huecos
// dos bloques vecinos en la lista van a estar uno al lado del otro en memoria

typedef struct Block {

    uint64_t size; // bytes del bloque que son usables (no contamos el header)
    uint64_t magic; // es o MAGIC_FREE o MAGIC_USED
    struct Block * next; // siguiente bloque en la lista
    struct Block * prev; // bloque anterior en la lista

} Block;