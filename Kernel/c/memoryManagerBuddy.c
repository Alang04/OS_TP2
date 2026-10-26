/**
 * Memory manager con el algoritmo "buddy system".
 * Solo se compila si se define USE_BUDDY (ver Kernel/Makefile).
 */

#ifdef USE_BUDDY

#include <stddef.h>
#include "memoryManager.h"

// bloque minimo de 2^6 = 64 bytes
#define MIN_ORDER 6

// cantidad de listas libres
#define MAX_ORDERS 48
#define ALIGNMENT 16

// usamos magic para informar si un bloque esta libre o ocupado
#define MAGIC_FREE 0xF3EEF3EEF3EEF3EEULL
#define MAGIC_USED 0xA110CA7EA110CA7EULL

// Todo bloque (libre u ocupado) empieza con magic y order. Los punteros next y
// prev solo se usan mientras el bloque esta en una lista libre: viven en el
// espacio que despues le toca al usuario.
typedef struct Block {
	uint64_t magic;
	uint64_t order;
	struct Block * next;
	struct Block * prev;
} Block;

#define HEADER_SIZE offsetof(Block, next)

static Block * freeLists[MAX_ORDERS];
static uint64_t heapStart = 0;
static uint64_t maxOrder = 0;       // el heap es un unico bloque de 2^maxOrder bytes
static uint64_t usedMemory = 0;     // bytes ocupados (bloques completos)
static uint64_t blocksUsed = 0;

static void pushFree(Block * b, uint64_t order);
static void removeFree(Block * b);
static Block * blockAt(uint64_t offset);

// El heap se recorta a la mayor potencia de 2 que entre en "size"
void memoryManagerInit(void * start, uint64_t size){
	for(int i = 0; i < MAX_ORDERS; i++){
		freeLists[i] = NULL;
	}
	usedMemory = 0;
	blocksUsed = 0;
	heapStart = 0;
	maxOrder = 0;

	uint64_t begin = ((uint64_t)start + (ALIGNMENT - 1)) & ~(uint64_t)(ALIGNMENT - 1);
	uint64_t lost = begin - (uint64_t)start;
	if(size <= lost){
		return;
	}
	size -= lost;

	uint64_t order = 0;
	while(order + 1 < MAX_ORDERS && ((uint64_t)1 << (order + 1)) <= size){
		order++;
	}
	if(order < MIN_ORDER){
		return;
	}

	heapStart = begin;
	maxOrder = order;
	pushFree(blockAt(0), maxOrder);
}

// Redondea el pedido (mas el header) a una potencia de 2 y parte bloques
// grandes hasta llegar a ese tamanio
void * memoryManagerAlloc(uint64_t size){
	if(heapStart == 0 || size == 0 || size > ((uint64_t)1 << maxOrder) - HEADER_SIZE){
		return NULL;
	}

	uint64_t order = MIN_ORDER;
	while(((uint64_t)1 << order) < size + HEADER_SIZE){
		order++;
	}

	uint64_t found = order;
	while(found <= maxOrder && freeLists[found] == NULL){
		found++;
	}
	if(found > maxOrder){
		return NULL;
	}

	Block * b = freeLists[found];
	removeFree(b);

	// Cada split deja la mitad de arriba libre y se sigue con la de abajo
	while(found > order){
		found--;
		Block * upper = (Block *)((uint64_t)b + ((uint64_t)1 << found));
		pushFree(upper, found);
	}

	b->magic = MAGIC_USED;
	b->order = order;
	usedMemory += (uint64_t)1 << order;
	blocksUsed++;
	return (void *)((uint64_t)b + HEADER_SIZE);
}

// Libera el bloque y lo fusiona con su buddy mientras este libre y del mismo tamanio
void memoryManagerFree(void * ptr){
	if(ptr == NULL || heapStart == 0){
		return;
	}

	uint64_t addr = (uint64_t)ptr;
	if(addr < heapStart + HEADER_SIZE || addr >= heapStart + ((uint64_t)1 << maxOrder)){
		return;
	}

	uint64_t offset = addr - HEADER_SIZE - heapStart;
	if(offset % ((uint64_t)1 << MIN_ORDER) != 0){
		return;
	}

	Block * b = blockAt(offset);
	if(b->magic != MAGIC_USED || b->order < MIN_ORDER || b->order > maxOrder){
		return; // puntero invalido o double free
	}

	uint64_t order = b->order;
	usedMemory -= (uint64_t)1 << order;
	blocksUsed--;

	while(order < maxOrder){
		uint64_t buddyOffset = offset ^ ((uint64_t)1 << order);
		Block * buddy = blockAt(buddyOffset);
		if(buddy->magic != MAGIC_FREE || buddy->order != order){
			break;
		}

		removeFree(buddy);
		if(buddyOffset < offset){
			offset = buddyOffset;
		}
		// el header del bloque absorbido queda adentro del bloque grande
		b->magic = 0;
		buddy->magic = 0;
		b = blockAt(offset);
		order++;
	}

	pushFree(b, order);
}

void memoryManagerGetStats(MemoryManagerStats * info){
	if(info == NULL){
		return;
	}

	info->totalMemory = heapStart == 0 ? 0 : ((uint64_t)1 << maxOrder);
	info->usedMemory = usedMemory;
	info->freeMemory = info->totalMemory - usedMemory;
	info->blocksUsed = blocksUsed;
}

static Block * blockAt(uint64_t offset){
	return (Block *)(heapStart + offset);
}

static void pushFree(Block * b, uint64_t order){
	b->magic = MAGIC_FREE;
	b->order = order;
	b->prev = NULL;
	b->next = freeLists[order];
	if(b->next != NULL){
		b->next->prev = b;
	}
	freeLists[order] = b;
}

static void removeFree(Block * b){
	if(b->prev != NULL){
		b->prev->next = b->next;
	} else {
		freeLists[b->order] = b->next;
	}
	if(b->next != NULL){
		b->next->prev = b->prev;
	}
}

#endif
