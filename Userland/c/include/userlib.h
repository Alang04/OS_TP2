// API de solo consumo (userland): wrappers de syscalls y utilitarios mínimos
// Contrato:
// - Todas las funciones sys_* invocan al kernel vía int 0x80.
// - sys_read bloquea (sin busy waiting) hasta que haya input. sys_read_timeout espera como
//   maximo 'timeout' ticks y devuelve 0 si no llego nada (con timeout 0 no espera).
// - sys_ticks devuelve ticks desde arranque; tiempo/fecha en BCD.
// - Redraw buffer registra salida para re-render al cambiar tamaño de fuente.
#ifndef USERLIB_H
#define USERLIB_H

#include <stdint.h>
#include <stddef.h>

#define STDOUT 1
#define STDERR 2
#define REGSBUFF 500
#define REDRAW_BUFF 4096
#define KB 1024
#define BM_BUFF 20
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define EIGHTH 50
#define QUARTER 100
#define HALF 200

typedef struct{
    char character;
    uint64_t fd;
}RedrawStruct;

#define MAX_PROCESS_NAME 32

typedef int64_t (*ProcessMain)(uint64_t argc, char ** argv);

// Debe coincidir con ProcessInfo de Kernel/include/process.h
typedef struct{
    int64_t pid;
    int64_t ppid;
    char name[MAX_PROCESS_NAME];
    uint8_t priority;
    uint8_t foreground;
    uint8_t state;
    uint64_t stackBase;
    uint64_t rsp;
    uint64_t rbp;
}ProcessInfo;

// Debe coincidir con MemoryManagerStats de Kernel/include/memoryManager.h
typedef struct{
    uint64_t totalMemory;
    uint64_t usedMemory;
    uint64_t freeMemory;
    uint64_t blocksUsed;
}MemoryInfo;

void redraw_reset(void);
void redraw_append_char(char c, uint64_t fd);
uint64_t sys_write(uint64_t fd, const char * buff, uint64_t count);
uint64_t sys_read(char * buff, uint64_t count);
uint64_t sys_read_timeout(char * buff, uint64_t count, uint64_t timeout);
uint64_t sys_registers(char * buff);
void sys_time(uint8_t * buff);
void sys_date(uint8_t * buff);
void sys_increase_fontsize(void);
void sys_decrease_fontsize(void);
void sys_beep(uint32_t freq, uint64_t time);
uint64_t sys_ticks(void);
void sys_clear(void);
void sys_speaker_start(uint32_t freq);
void sys_speaker_off(void);
uint64_t sys_screen_width(void);
uint64_t sys_screen_height(void);
void sys_putpixel(uint32_t color, uint64_t x, uint64_t y);
void sys_fill_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t color);
void * sys_malloc(uint64_t size);
void sys_free(void * ptr);
int64_t sys_create_process(ProcessMain entry, const char * name, uint64_t argc, char ** argv, uint8_t foreground);
void sys_exit(int64_t retValue);
int64_t sys_getpid(void);
uint64_t sys_ps(ProcessInfo * buff, uint64_t max);
int64_t sys_kill(int64_t pid);
int64_t sys_nice(int64_t pid, uint8_t priority);
int64_t sys_block(int64_t pid);
int64_t sys_unblock(int64_t pid);
void sys_yield(void);
int64_t sys_waitpid(int64_t pid);
int64_t sys_sem_open(const char * name, uint64_t initialValue);
int64_t sys_sem_wait(const char * name);
int64_t sys_sem_post(const char * name);
int64_t sys_sem_close(const char * name);
int64_t sys_mem_info(MemoryInfo * info);
int64_t sys_wait_nohang(void);

// Mini libc usada por los tests de la catedra (c/libc.c; memset en _loader.c)
void * malloc(size_t size);
void free(void * ptr);
void * memset(void * dest, int value, size_t count);
int printf(const char * fmt, ...);
uint64_t putchar(char c);
char getchar(void);
void processLine(char * buff, uint32_t * history_len);
uint64_t num_to_str(uint64_t value, char * dest, int base);
void gen_invalid_opcode(void);
void help(void);
void clear(void);
void registers(void);
void divideByZero(void);
void printTime(void);
void printDate(void);
void playBeep(void);
void invOp(void);
uint8_t adjustHour(uint8_t hour, int offset);
void printTimeAndDate(uint8_t* buff, char separator);
void shellIncreaseFontSize(void);
void shellDecreaseFontSize(void);
void redrawFont(void);
void bmMEM(void);
void bmCPU(void);
void bmFPS(void);
void bmKEY(void);

#endif
