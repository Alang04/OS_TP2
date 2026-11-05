#include "syscallDispatcher.h"
#include "videoDriver.h"
#include "keyboardDriver.h"
#include "lib.h"
#include "defs.h"
#include "time.h"
#include "sound.h"
#include "memoryManager.h"
#include "process.h"
#include "scheduler.h"
#include "semaphore.h"

// Aumenta el tamaño de fuente por defecto
void sys_increase_fontsize(){
    increaseFontSize();
}
void sys_decrease_fontsize(){
    decreaseFontSize();
}

// Limpia la pantalla en modo gráfico (negro)
void sys_clear(){
    clearScreen(0x000000);
}

// Escribe 'count' bytes en la salida indicada (texto gráfico)
uint64_t sys_write(uint64_t fd, const char * buff, uint64_t count){
    (void)fd;
    uint32_t color = 0xFFFFFF;

    for(uint64_t i = 0; i < count; i++){
        videoPutChar((uint8_t)buff[i], color);
    }

    return count;
}

uint64_t sys_read(char * buff, uint64_t count){
   return readKeyBuff(buff, count);
}

void sys_date(uint8_t * buff){
    date(buff);
}

void sys_time(uint8_t * buff){
    time(buff);
}

uint64_t sys_registers(char * buff){
    return copyRegistersBuffer(buff);
}

void sys_beep(uint32_t freq, uint64_t time){
    beep(freq, time);
}

// Retorna el contador de ticks desde el arranque
uint64_t sys_ticks(){
    return deltaTicks();
}

void sys_speaker_start(uint32_t freq){
    startSpeaker(freq);
}

void sys_speaker_off(){
    turnOff();
}

uint64_t sys_screen_width(){
    return (uint64_t)getScreenWidth();
}

uint64_t sys_screen_height(){
    return (uint64_t)getScreenHeight();
}

void sys_putpixel(uint32_t color, uint64_t x, uint64_t y){
    putPixel(color, x, y);
}

void sys_fill_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t color){
    fillRect(x, y, w, h, color);
}

// Reserva 'size' bytes del heap, devuelve NULL si no hay memoria suficiente
void * sys_malloc(uint64_t size){
    return memoryManagerAlloc(size);
}

// Libera un bloque reservado con sys_malloc
void sys_free(void * ptr){
    memoryManagerFree(ptr);
}

int64_t sys_create_process(ProcessMain entry, const char * name, uint64_t argc, char ** argv, uint8_t foreground){
    return createProcess(entry, name, argc, argv, foreground);
}

void sys_exit(int64_t retValue){
    exitProcess(retValue);
}

int64_t sys_getpid(void){
    return getPid();
}

uint64_t sys_ps(ProcessInfo * buff, uint64_t max){
    return listProcesses(buff, max);
}

int64_t sys_kill(int64_t pid){
    return killProcess(pid);
}

int64_t sys_nice(int64_t pid, uint8_t priority){
    return setPriority(pid, priority);
}

int64_t sys_block(int64_t pid){
    return blockProcess(pid);
}

int64_t sys_unblock(int64_t pid){
    return unblockProcess(pid);
}

void sys_yield(void){
    forceSwitch();
}

int64_t sys_waitpid(int64_t pid){
    return waitProcess(pid);
}

int64_t sys_sem_open(const char * name, uint64_t initialValue){
    return semOpen(name, initialValue);
}

int64_t sys_sem_wait(const char * name){
    return semWait(name);
}

int64_t sys_sem_post(const char * name){
    return semPost(name);
}

int64_t sys_sem_close(const char * name){
    return semClose(name);
}

// Copia en 'info' el estado del memory manager (total, ocupada, libre y bloques)
int64_t sys_mem_info(MemoryManagerStats * info){
    if(info == 0){
        return -1;
    }
    memoryManagerGetStats(info);
    return 0;
}

void * syscalls[CANT_SYS] = {
    &sys_registers,         // 0
    &sys_time,              // 1
    &sys_date,              // 2
    &sys_read,              // 3
    &sys_write,             // 4
    &sys_increase_fontsize, // 5
    &sys_decrease_fontsize, // 6
    &sys_beep,              // 7
    &sys_ticks,             // 8
    &sys_clear,             // 9
    &sys_speaker_start,     // 10
    &sys_speaker_off,       // 11
    &sys_screen_width,      // 12
    &sys_screen_height,     // 13
    &sys_putpixel,          // 14
    &sys_fill_rect,         // 15
    &sys_malloc,            // 16
    &sys_free,              // 17
    &sys_create_process,    // 18
    &sys_exit,              // 19
    &sys_getpid,            // 20
    &sys_ps,                // 21
    &sys_kill,              // 22
    &sys_nice,              // 23
    &sys_block,             // 24
    &sys_unblock,           // 25
    &sys_yield,             // 26
    &sys_waitpid,           // 27
    &sys_sem_open,          // 28
    &sys_sem_wait,          // 29
    &sys_sem_post,          // 30
    &sys_sem_close,         // 31
    &sys_mem_info           // 32
};