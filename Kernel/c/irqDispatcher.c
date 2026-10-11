#include <stdint.h>
#include "time.h"
#include "keyboardDriver.h"
#include "irqDispatcher.h"

static void int_20(void);

// Enruta IRQs del PIC a sus manejadores de C
void irqDispatcher(uint64_t irq){
	switch(irq){
		case 0:
			int_20();
			break;
		case 1:
			handlePressedKey();
			break;
	}
	return;
}

// IRQ0: tick del PIT
void int_20(void){
	timer_handler();
}