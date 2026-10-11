#include <stdint.h>

extern char bss;
extern char endOfBinary;
int main(void);
int _start(void);

void * memset(void * destiny, int32_t c, uint64_t length);

// Entry de módulo de usuario: limpia BSS y salta a main
int _start(void){
	memset(&bss, 0, &endOfBinary - &bss);
	return main();
}

// Implementación local de memset para entorno freestanding
void * memset(void * destination, int32_t c, uint64_t length){
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destination;

	while(length--){
		dst[length] = chr;
	}

	return destination;
}
