// Tabla de procesos y ciclo de vida: creacion, terminacion, bloqueo y espera.
// Todo corre en ring 0 y en un unico espacio de direcciones, asi que cada proceso
// usa un solo stack tanto para su codigo como para las syscalls que ejecuta.
// Las funciones publicas se llaman desde syscalls (interrupciones deshabilitadas).
#include "process.h"
#include "scheduler.h"
#include "memoryManager.h"
#include "interrupts.h"
#include "lib.h"

#define NO_PARENT (-1)
#define KERNEL_CS 0x08
#define KERNEL_SS 0x00
#define INITIAL_RFLAGS 0x202    // IF=1: el proceso arranca con interrupciones habilitadas
#define STACK_ALIGN_MASK (~(uint64_t)0xF)

// Layout que dejan en el stack la interrupcion + pushState (desde rsp hacia arriba)
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss;
} StackFrame;

static PCB processes[MAX_PROCESSES];
static int64_t nextPid = IDLE_PID;
static int64_t shellPid = NO_PID;
static ProcessMain shellEntry = 0;

PCB * getProcessTable(void){
    return processes;
}

static uint64_t strLength(const char * s){
    uint64_t n = 0;
    while(s[n] != '\0'){
        n++;
    }
    return n;
}

static void copyName(char * dest, const char * src){
    uint64_t i = 0;
    if(src != 0){
        for(; i < MAX_NAME_LENGTH - 1 && src[i] != '\0'; i++){
            dest[i] = src[i];
        }
    }
    dest[i] = '\0';
}

// Copia argv (punteros + strings) a un unico bloque terminado en NULL
static char ** copyArgs(uint64_t argc, char ** argv){
    uint64_t size = (argc + 1) * sizeof(char *);
    for(uint64_t i = 0; i < argc; i++){
        size += strLength(argv[i]) + 1;
    }

    char ** copy = memoryManagerAlloc(size);
    if(copy == 0){
        return 0;
    }

    char * strings = (char *)(copy + argc + 1);
    for(uint64_t i = 0; i < argc; i++){
        uint64_t len = strLength(argv[i]) + 1;
        memcpy(strings, argv[i], len);
        copy[i] = strings;
        strings += len;
    }
    copy[argc] = 0;

    return copy;
}

static PCB * findProcess(int64_t pid){
    for(int i = 0; i < MAX_PROCESSES; i++){
        if(processes[i].state != FREE && processes[i].pid == pid){
            return &processes[i];
        }
    }
    return 0;
}

static PCB * findFreeSlot(void){
    for(int i = 0; i < MAX_PROCESSES; i++){
        if(processes[i].state == FREE){
            return &processes[i];
        }
    }
    return 0;
}

static void freeProcess(PCB * p){
    memoryManagerFree(p->stackBase);
    memoryManagerFree(p->argv);
    p->state = FREE;
}

// Libera los zombies que ya no tienen padre que los espere.
// Nunca libera al proceso actual: su stack es el que se esta usando.
static void reapOrphans(void){
    PCB * current = getCurrentProcess();
    for(int i = 0; i < MAX_PROCESSES; i++){
        PCB * p = &processes[i];
        if(p->state == ZOMBIE && p->ppid == NO_PARENT && p != current){
            freeProcess(p);
        }
    }
}

// Pasa a ZOMBIE: los hijos quedan huerfanos y se despierta al padre si lo esperaba
static void markZombie(PCB * p, int64_t retValue){
    p->state = ZOMBIE;
    p->blockReason = BLOCK_NONE;
    p->retValue = retValue;

    for(int i = 0; i < MAX_PROCESSES; i++){
        if(processes[i].state != FREE && processes[i].ppid == p->pid){
            processes[i].ppid = NO_PARENT;
        }
    }

    PCB * parent = findProcess(p->ppid);
    if(parent != 0 && parent->state == BLOCKED && parent->blockReason == BLOCK_WAIT
        && parent->waitingFor == p->pid){
        parent->state = READY;
        parent->blockReason = BLOCK_NONE;
    }
}

// Primer codigo que ejecuta todo proceso: corre su main y lo termina con su retorno
static void processStart(ProcessMain entry, uint64_t argc, char ** argv){
    int64_t ret = entry(argc, argv);
    _cli();
    exitProcess(ret);
}

static int64_t idleMain(uint64_t argc, char ** argv){
    while(1){
        _hlt();
    }
    return 0;
}

void initProcesses(void){
    createProcess(idleMain, "idle", 0, 0, 0);
    processes[0].priority = MIN_PRIORITY;
}

int64_t createProcess(ProcessMain entry, const char * name, uint64_t argc, char ** argv, uint8_t foreground){
    if(entry == 0 || argc > MAX_ARGS || (argc > 0 && argv == 0)){
        return -1;
    }

    reapOrphans();

    PCB * p = findFreeSlot();
    if(p == 0){
        return -1;
    }

    void * stack = memoryManagerAlloc(PROCESS_STACK_SIZE);
    if(stack == 0){
        return -1;
    }

    char ** args = copyArgs(argc, argv);
    if(args == 0){
        memoryManagerFree(stack);
        return -1;
    }

    PCB * parent = getCurrentProcess();
    p->pid = nextPid++;
    p->ppid = parent != 0 ? parent->pid : NO_PARENT;
    copyName(p->name, name);
    p->priority = DEFAULT_PRIORITY;
    p->foreground = foreground;
    p->blockReason = BLOCK_NONE;
    p->waitingFor = NO_PID;
    p->retValue = 0;
    p->wakeTick = 0;
    p->stackBase = stack;
    p->argv = args;

    // Stack inicial falso: el primer popState + iretq "vuelve" a processStart(entry, argc, args)
    uint64_t top = ((uint64_t)stack + PROCESS_STACK_SIZE) & STACK_ALIGN_MASK;
    StackFrame * frame = (StackFrame *)(top - sizeof(StackFrame));
    memset(frame, 0, sizeof(StackFrame));
    frame->rdi = (uint64_t)entry;
    frame->rsi = argc;
    frame->rdx = (uint64_t)args;
    frame->rip = (uint64_t)processStart;
    frame->cs = KERNEL_CS;
    frame->rflags = INITIAL_RFLAGS;
    frame->rsp = top - sizeof(uint64_t);   // como si un call hubiese pusheado la direccion de retorno
    frame->ss = KERNEL_SS;

    p->rsp = (uint64_t)frame;
    p->state = READY;

    return p->pid;
}

int64_t createShell(ProcessMain entry){
    shellEntry = entry;
    shellPid = createProcess(entry, "sh", 0, 0, 1);
    return shellPid;
}

void exitProcess(int64_t retValue){
    markZombie(getCurrentProcess(), retValue);
    forceSwitch();
    while(1){
        _hlt();     // no deberia volver a ejecutarse nunca
    }
}

int64_t getPid(void){
    PCB * current = getCurrentProcess();
    return current != 0 ? current->pid : NO_PID;
}

int64_t killProcess(int64_t pid){
    if(pid == IDLE_PID || pid == shellPid){
        return -1;
    }

    PCB * p = findProcess(pid);
    if(p == 0 || p->state == ZOMBIE){
        return -1;
    }

    markZombie(p, -1);
    if(p == getCurrentProcess()){
        forceSwitch();
    }
    return 0;
}

int64_t setPriority(int64_t pid, uint8_t priority){
    PCB * p = findProcess(pid);
    if(p == 0 || p->state == ZOMBIE || pid == IDLE_PID || priority > MAX_PRIORITY){
        return -1;
    }

    p->priority = priority;
    return 0;
}

int64_t blockProcess(int64_t pid){
    PCB * p = findProcess(pid);
    if(p == 0 || p->state == ZOMBIE || pid == IDLE_PID){
        return -1;
    }

    if(p->state == BLOCKED){
        return p->blockReason == BLOCK_USER ? 0 : -1;
    }

    p->state = BLOCKED;
    p->blockReason = BLOCK_USER;
    if(p == getCurrentProcess()){
        forceSwitch();
    }
    return 0;
}

int64_t unblockProcess(int64_t pid){
    PCB * p = findProcess(pid);
    if(p == 0 || p->state == ZOMBIE){
        return -1;
    }

    if(p->state == BLOCKED){
        if(p->blockReason != BLOCK_USER){
            return -1;
        }
        p->state = READY;
        p->blockReason = BLOCK_NONE;
    }
    return 0;
}

void blockCurrentProcess(BlockReason reason){
    PCB * current = getCurrentProcess();
    current->state = BLOCKED;
    current->blockReason = reason;
    forceSwitch();
}

void blockCurrentProcessUntil(BlockReason reason, uint64_t wakeTick){
    PCB * current = getCurrentProcess();
    current->wakeTick = wakeTick;
    blockCurrentProcess(reason);
    current->wakeTick = 0;
}

int64_t wakeProcess(int64_t pid, BlockReason reason){
    PCB * p = findProcess(pid);
    if(p == 0 || p->state != BLOCKED || p->blockReason != reason){
        return -1;
    }

    p->state = READY;
    p->blockReason = BLOCK_NONE;
    return 0;
}

void wakeAllProcesses(BlockReason reason){
    for(int i = 0; i < MAX_PROCESSES; i++){
        PCB * p = &processes[i];
        if(p->state == BLOCKED && p->blockReason == reason){
            p->state = READY;
            p->blockReason = BLOCK_NONE;
            p->wakeTick = 0;    // si no, un block posterior lo despertaria por tiempo
        }
    }
}

void wakeExpiredProcesses(uint64_t now){
    for(int i = 0; i < MAX_PROCESSES; i++){
        PCB * p = &processes[i];
        if(p->state == BLOCKED && p->wakeTick != 0 && now >= p->wakeTick){
            p->state = READY;
            p->blockReason = BLOCK_NONE;
            p->wakeTick = 0;
        }
    }
}

int64_t waitProcess(int64_t pid){
    PCB * current = getCurrentProcess();
    PCB * child = findProcess(pid);
    if(current == 0 || child == 0 || child->ppid != current->pid){
        return -1;
    }

    if(child->state != ZOMBIE){
        current->state = BLOCKED;
        current->blockReason = BLOCK_WAIT;
        current->waitingFor = pid;
        forceSwitch();      // markZombie del hijo nos vuelve a poner READY
        current->waitingFor = NO_PID;
    }

    int64_t retValue = child->retValue;
    freeProcess(child);
    reapOrphans();
    return retValue;
}

int64_t waitAnyChildNoHang(void){
    PCB * current = getCurrentProcess();
    if(current == 0){
        return -1;
    }

    for(int i = 0; i < MAX_PROCESSES; i++){
        PCB * p = &processes[i];
        if(p->state == ZOMBIE && p->ppid == current->pid){
            int64_t pid = p->pid;
            freeProcess(p);
            return pid;
        }
    }
    return -1;
}

uint64_t listProcesses(ProcessInfo * buff, uint64_t max){
    uint64_t count = 0;
    for(int i = 0; i < MAX_PROCESSES && count < max; i++){
        PCB * p = &processes[i];
        if(p->state == FREE){
            continue;
        }

        ProcessInfo * info = &buff[count++];
        info->pid = p->pid;
        info->ppid = p->ppid;
        copyName(info->name, p->name);
        info->priority = p->priority;
        info->foreground = p->foreground;
        info->state = (uint8_t)p->state;
        info->stackBase = ((uint64_t)p->stackBase + PROCESS_STACK_SIZE) & STACK_ALIGN_MASK;
        info->rsp = p->rsp;
        info->rbp = ((StackFrame *)p->rsp)->rbp;   // contexto guardado en el ultimo cambio
    }
    return count;
}

void killCurrentOnException(void){
    _cli();
    PCB * current = getCurrentProcess();
    if(current == 0){
        haltcpu();  // excepcion antes de iniciar el primer proceso
    }

    uint8_t wasShell = current->pid == shellPid;
    markZombie(current, -1);
    if(wasShell){
        createShell(shellEntry);
    }

    forceSwitch();
}
