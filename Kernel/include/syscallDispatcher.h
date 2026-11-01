#ifndef SYSCALLDISPATCHER_H
#define SYSCALLDISPATCHER_H

#include <stdint.h>
#include "defs.h"
#include "process.h"

extern void * syscalls[CANT_SYS];

uint64_t sys_write(uint64_t fd, const char * buff, uint64_t count);
uint64_t sys_read(char * buff, uint64_t count);
uint64_t sys_registers(char * buff);
void sys_time(uint8_t * buff);
void sys_date(uint8_t * buff);
void sys_decrease_fontsize(void);
void sys_increase_fontsize(void);
void sys_beep(uint32_t freq, uint64_t time);
uint64_t sys_ticks(void);
void sys_clear(void);
uint64_t sys_screen_width(void);
uint64_t sys_screen_height(void);
void sys_putpixel(uint32_t color, uint64_t x, uint64_t y);
void sys_fill_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t color);
void sys_speaker_start(uint32_t freq);
void sys_speaker_off(void);
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

#endif