// Wrappers que usan los tests de la catedra.
// TODO: conectar con las syscalls de procesos, scheduling y semaforos cuando
// esten implementadas en el kernel. Mientras tanto devuelven -1 (error).
#include <stdint.h>
#include "syscall.h"

int64_t my_getpid(void) {
  return -1;
}

int64_t my_create_process(char *name, uint64_t argc, char *argv[]) {
  return -1;
}

int64_t my_nice(uint64_t pid, uint64_t newPrio) {
  return -1;
}

int64_t my_kill(uint64_t pid) {
  return -1;
}

int64_t my_block(uint64_t pid) {
  return -1;
}

int64_t my_unblock(uint64_t pid) {
  return -1;
}

int64_t my_sem_open(char *sem_id, uint64_t initialValue) {
  return 0;
}

int64_t my_sem_wait(char *sem_id) {
  return -1;
}

int64_t my_sem_post(char *sem_id) {
  return -1;
}

int64_t my_sem_close(char *sem_id) {
  return -1;
}

int64_t my_yield(void) {
  return -1;
}

int64_t my_wait(int64_t pid) {
  return -1;
}
