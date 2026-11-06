#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define MAX_PROCESSES 64
#define PROCESS_STACK_SIZE 0x4000   // 16 KB: el proceso ejecuta sus syscalls en este mismo stack
#define MAX_NAME_LENGTH 32
#define MAX_ARGS 16

#define MIN_PRIORITY 0
#define MAX_PRIORITY 4
#define DEFAULT_PRIORITY 2

#define NO_PID (-1)
#define IDLE_PID 0

typedef int64_t (*ProcessMain)(uint64_t argc, char ** argv);

typedef enum { FREE = 0, READY, RUNNING, BLOCKED, ZOMBIE } ProcessState;

// Motivo del bloqueo: block/unblock de usuario solo actua sobre BLOCK_USER
typedef enum { BLOCK_NONE = 0, BLOCK_USER, BLOCK_WAIT, BLOCK_SEM, BLOCK_KEYBOARD } BlockReason;

typedef struct {
    int64_t pid;
    int64_t ppid;
    char name[MAX_NAME_LENGTH];
    uint8_t priority;
    uint8_t foreground;
    ProcessState state;
    BlockReason blockReason;
    int64_t waitingFor;     // pid del hijo esperado (BLOCK_WAIT)
    int64_t retValue;       // valor de retorno una vez ZOMBIE
    uint64_t wakeTick;      // tick en el que se lo despierta aunque siga bloqueado (0 = nunca)
    void * stackBase;       // bloque reservado para el stack (direccion mas baja)
    uint64_t rsp;           // rsp guardado en el ultimo cambio de contexto
    char ** argv;           // copia de los argumentos (un solo bloque)
} PCB;

// Informacion que se copia a userland (ps). Debe coincidir con userlib.h
typedef struct {
    int64_t pid;
    int64_t ppid;
    char name[MAX_NAME_LENGTH];
    uint8_t priority;
    uint8_t foreground;
    uint8_t state;
    uint64_t stackBase;     // tope del stack (donde empieza a crecer)
    uint64_t rsp;
    uint64_t rbp;
} ProcessInfo;

// Crea el proceso idle. Debe llamarse con interrupciones deshabilitadas
void initProcesses(void);

// Crea un proceso que ejecuta entry(argc, argv). Devuelve su pid o -1
int64_t createProcess(ProcessMain entry, const char * name, uint64_t argc, char ** argv, uint8_t foreground);

// Crea la shell y recuerda su entry para relanzarla si muere por una excepcion
int64_t createShell(ProcessMain entry);

// Termina el proceso actual (no retorna)
void exitProcess(int64_t retValue);

int64_t getPid(void);
int64_t killProcess(int64_t pid);
int64_t setPriority(int64_t pid, uint8_t priority);
int64_t blockProcess(int64_t pid);
int64_t unblockProcess(int64_t pid);

// Bloquea al proceso actual por 'reason' y cede el CPU. Vuelve cuando lo despiertan
void blockCurrentProcess(BlockReason reason);

// Como blockCurrentProcess, pero el timer lo despierta al llegar a 'wakeTick' (0 = sin limite)
void blockCurrentProcessUntil(BlockReason reason, uint64_t wakeTick);

// Despierta a 'pid' solo si esta bloqueado por 'reason'. Devuelve 0 si lo desperto, -1 si no
int64_t wakeProcess(int64_t pid, BlockReason reason);

// Despierta a todos los procesos bloqueados por 'reason'
void wakeAllProcesses(BlockReason reason);

// Despierta a los procesos cuyo wakeTick ya paso. Se llama en cada tick del timer
void wakeExpiredProcesses(uint64_t now);

// Espera a que termine el hijo 'pid' y devuelve su valor de retorno (-1 si no es hijo)
int64_t waitProcess(int64_t pid);

// Libera un hijo ZOMBIE cualquiera sin bloquearse (como waitpid(-1, WNOHANG)).
// Devuelve su pid, o -1 si ningun hijo termino
int64_t waitAnyChildNoHang(void);

// Copia hasta 'max' procesos en buff, devuelve cuantos copio
uint64_t listProcesses(ProcessInfo * buff, uint64_t max);

// Mata al proceso que provoco una excepcion (no retorna)
void killCurrentOnException(void);

PCB * getProcessTable(void);

#endif
