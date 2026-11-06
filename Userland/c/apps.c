// Aplicaciones de userland: wrappers de las syscalls de memoria y procesos
#include <stdint.h>
#include "userlib.h"
#include "apps.h"

#define MAX_PS_PROCESSES 64     // MAX_PROCESSES del kernel
#define TICKS_PER_SECOND 18     // el PIT queda en su frecuencia por defecto (~18.2 Hz)
#define MAX_USER_PRIORITY 4     // MAX_PRIORITY del kernel
#define BLOCKED_STATE 3         // ProcessState BLOCKED del kernel
#define NUM_BUFF 24

// Mismo orden que ProcessState de Kernel/include/process.h
static const char * stateNames[] = {"FREE", "READY", "RUNNING", "BLOCKED", "ZOMBIE"};
#define STATE_COUNT (sizeof(stateNames) / sizeof(stateNames[0]))

// Convierte 'str' en un entero no negativo. Devuelve 0 si no es un numero valido
static int parseNumber(const char * str, int64_t * out){
    if(str == 0 || *str == '\0'){
        return 0;
    }

    int64_t value = 0;
    for(; *str != '\0'; str++){
        if(*str < '0' || *str > '9'){
            return 0;
        }
        value = value * 10 + (*str - '0');
    }

    *out = value;
    return 1;
}

// Imprime 'str' y completa con espacios hasta ocupar 'width' columnas
static void printColumn(const char * str, int width){
    int written = printf("%s", str);
    for(; written < width; written++){
        putchar(' ');
    }
}

static void printNumberColumn(uint64_t value, int width){
    char buff[NUM_BUFF];
    num_to_str(value, buff, 10);
    printColumn(buff, width);
}

static void printHexColumn(uint64_t value, int width){
    char buff[NUM_BUFF + 2] = "0x";
    num_to_str(value, buff + 2, 16);
    printColumn(buff, width);
}

static void printMemoryLine(const char * label, uint64_t bytes){
    char buff[NUM_BUFF];
    printColumn(label, 10);
    num_to_str(bytes, buff, 10);
    printf("%s bytes (", buff);
    num_to_str(bytes / KB, buff, 10);
    printf("%s KB)\n", buff);
}

// Copia la tabla de procesos del kernel. Devuelve la cantidad o -1 si no hay memoria
static int64_t getProcesses(ProcessInfo ** list){
    *list = malloc(MAX_PS_PROCESSES * sizeof(ProcessInfo));
    if(*list == 0){
        return -1;
    }
    return (int64_t)sys_ps(*list, MAX_PS_PROCESSES);
}

int64_t mem(uint64_t argc, char ** argv){
    if(argc != 0){
        return -1;
    }

    MemoryInfo info;
    if(sys_mem_info(&info) != 0){
        printf("mem: no se pudo obtener el estado de la memoria\n");
        return 1;
    }

    printMemoryLine("Total:", info.totalMemory);
    printMemoryLine("Ocupada:", info.usedMemory);
    printMemoryLine("Libre:", info.freeMemory);

    char buff[NUM_BUFF];
    num_to_str(info.blocksUsed, buff, 10);
    printf("Bloques ocupados: %s\n", buff);
    return 0;
}

int64_t ps(uint64_t argc, char ** argv){
    if(argc != 0){
        return -1;
    }

    ProcessInfo * list;
    int64_t count = getProcesses(&list);
    if(count < 0){
        printf("ps: no hay memoria suficiente\n");
        return 1;
    }

    printColumn("PID", 6);
    printColumn("PPID", 6);
    printColumn("PRIO", 6);
    printColumn("ESTADO", 9);
    printColumn("FG", 4);
    printColumn("STACK", 12);
    printColumn("RSP", 12);
    printColumn("RBP", 12);
    printf("NOMBRE\n");

    for(int64_t i = 0; i < count; i++){
        ProcessInfo * p = &list[i];
        printNumberColumn((uint64_t)p->pid, 6);
        if(p->ppid < 0){
            printColumn("-", 6);
        } else {
            printNumberColumn((uint64_t)p->ppid, 6);
        }
        printNumberColumn(p->priority, 6);
        printColumn(p->state < STATE_COUNT ? stateNames[p->state] : "?", 9);
        printColumn(p->foreground ? "si" : "no", 4);
        printHexColumn(p->stackBase, 12);
        printHexColumn(p->rsp, 12);
        printHexColumn(p->rbp, 12);
        printf("%s\n", p->name);
    }

    free(list);
    return 0;
}

// Saluda cada 'segundos' (1 por defecto). La espera es activa: no se bloquea
int64_t loop(uint64_t argc, char ** argv){
    int64_t seconds = 1;
    if(argc > 1 || (argc == 1 && (!parseNumber(argv[0], &seconds) || seconds == 0))){
        return -1;
    }

    int64_t pid = sys_getpid();
    uint64_t interval = (uint64_t)seconds * TICKS_PER_SECOND;

    while(1){
        printf("Hola! Soy el proceso %d\n", (int)pid);
        uint64_t start = sys_ticks();
        while(sys_ticks() - start < interval){
            ;
        }
    }

    return 0;
}

int64_t kill(uint64_t argc, char ** argv){
    int64_t pid;
    if(argc != 1 || !parseNumber(argv[0], &pid)){
        return -1;
    }

    if(sys_kill(pid) != 0){
        printf("kill: no se pudo matar al proceso %d\n", (int)pid);
        return 1;
    }
    return 0;
}

int64_t nice(uint64_t argc, char ** argv){
    int64_t pid;
    int64_t priority;
    if(argc != 2 || !parseNumber(argv[0], &pid) || !parseNumber(argv[1], &priority)
        || priority > MAX_USER_PRIORITY){
        return -1;
    }

    if(sys_nice(pid, (uint8_t)priority) != 0){
        printf("nice: no se pudo cambiar la prioridad del proceso %d\n", (int)pid);
        return 1;
    }
    printf("Proceso %d ahora tiene prioridad %d\n", (int)pid, (int)priority);
    return 0;
}

// Alterna el proceso entre bloqueado y listo segun su estado actual
int64_t block(uint64_t argc, char ** argv){
    int64_t pid;
    if(argc != 1 || !parseNumber(argv[0], &pid)){
        return -1;
    }

    ProcessInfo * list;
    int64_t count = getProcesses(&list);
    if(count < 0){
        printf("block: no hay memoria suficiente\n");
        return 1;
    }

    int64_t state = -1;
    for(int64_t i = 0; i < count; i++){
        if(list[i].pid == pid){
            state = list[i].state;
        }
    }
    free(list);

    if(state < 0){
        printf("block: no existe el proceso %d\n", (int)pid);
        return 1;
    }

    if(state == BLOCKED_STATE){
        if(sys_unblock(pid) != 0){
            // Bloqueado por el sistema (semaforo o esperando un hijo), no por block
            printf("block: no se pudo desbloquear al proceso %d\n", (int)pid);
            return 1;
        }
        printf("Proceso %d desbloqueado\n", (int)pid);
    } else {
        if(sys_block(pid) != 0){
            printf("block: no se pudo bloquear al proceso %d\n", (int)pid);
            return 1;
        }
        printf("Proceso %d bloqueado\n", (int)pid);
    }
    return 0;
}
