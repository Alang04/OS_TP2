#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "process.h"

// Habilita el scheduler (antes de esto los ticks no cambian de contexto)
void initScheduler(void);

// Llamado desde el timer: reciben el rsp del proceso interrumpido y devuelven el del proximo
uint64_t scheduleTick(uint64_t rsp);

// Llamado desde la interrupcion de yield (int 0x81): cambia de proceso inmediatamente
uint64_t scheduleYield(uint64_t rsp);

// Proceso en ejecucion (NULL antes del primer cambio de contexto)
PCB * getCurrentProcess(void);

// Cede el CPU disparando int 0x81 (asm/interrupts.asm). Sirve con interrupciones deshabilitadas
void forceSwitch(void);

#endif
