#ifndef KEYBOARDDRIVER_H
#define KEYBOARDDRIVER_H

#include <defs.h>
#include <stdint.h>

void writeBuff(unsigned char c);
void clearBuff(void);
void handlePressedKey(void);
uint64_t copyRegistersBuffer(char * buff);
uint8_t getFromBuffer(void);
uint64_t readKeyBuff(char * buff, uint64_t count);

#define NO_TIMEOUT ((uint64_t)-1)

// Lee hasta 'count' bytes; si no hay ninguno espera hasta 'timeout' ticks
// (NO_TIMEOUT: sin limite, 0: no espera). Devuelve cuantos leyo
uint64_t readKeyboard(char * buff, uint64_t count, uint64_t timeout);
uint32_t intToHexa(uint64_t value, char * dest);
extern uint64_t regsArray[];
uint8_t kbd_scancode_read(void);

#endif