// Round Robin con prioridades: cada proceso corre (prioridad + 1) ticks seguidos
// y luego se elige al siguiente READY de la tabla en orden circular.
// El idle solo corre cuando no hay ningun otro proceso listo.
#include "scheduler.h"

static PCB * current = 0;
static uint64_t quantumLeft = 0;
static uint8_t enabled = 0;

void initScheduler(void){
    enabled = 1;
}

PCB * getCurrentProcess(void){
    return current;
}

static uint64_t quantumFor(uint8_t priority){
    return (uint64_t)priority + 1;
}

// Siguiente READY despues del actual; el actual (si sigue READY) queda ultimo
static PCB * nextReady(void){
    PCB * table = getProcessTable();
    uint64_t start = current != 0 ? (uint64_t)(current - table) : 0;
    PCB * idle = 0;

    for(uint64_t i = 1; i <= MAX_PROCESSES; i++){
        PCB * p = &table[(start + i) % MAX_PROCESSES];
        if(p->state != READY){
            continue;
        }
        if(p->pid == IDLE_PID){
            idle = p;
            continue;
        }
        return p;
    }

    return idle;
}

static uint64_t switchProcess(uint64_t rsp){
    // Antes del primer cambio no hay proceso actual: el contexto de arranque se descarta
    if(current != 0){
        current->rsp = rsp;
        if(current->state == RUNNING){
            current->state = READY;
        }
    }

    PCB * next = nextReady();
    if(next == 0){
        if(current != 0 && current->state == READY){
            current->state = RUNNING;
        }
        return rsp;
    }

    current = next;
    current->state = RUNNING;
    quantumLeft = quantumFor(current->priority);
    return current->rsp;
}

uint64_t scheduleTick(uint64_t rsp){
    if(!enabled){
        return rsp;
    }

    if(current != 0 && current->state == RUNNING && quantumLeft > 1){
        quantumLeft--;
        return rsp;
    }

    return switchProcess(rsp);
}

uint64_t scheduleYield(uint64_t rsp){
    if(!enabled){
        return rsp;
    }

    return switchProcess(rsp);
}
