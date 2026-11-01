// Wrappers que usan los tests de la catedra sobre las syscalls del kernel.
// TODO: conectar los semaforos (fase 2). Mientras tanto my_sem_open falla.
#include <stdint.h>
#include "userlib.h"
#include "syscall.h"
#include "test_util.h"
#include "tests.h"

// Los tests crean procesos por nombre: se traducen a su funcion de entrada
typedef struct {
  char *name;
  ProcessMain entry;
} TestProcess;

static TestProcess testProcesses[] = {
    {"endless_loop", (ProcessMain)endless_loop},
    {"zero_to_max", (ProcessMain)zero_to_max},
    {"my_process_inc", my_process_inc},
    {0, 0},
};

static int streq(const char *a, const char *b) {
  while (*a && *a == *b) {
    a++;
    b++;
  }
  return *a == *b;
}

int64_t my_getpid(void) {
  return sys_getpid();
}

int64_t my_create_process(char *name, uint64_t argc, char *argv[]) {
  for (int i = 0; testProcesses[i].name != 0; i++)
    if (streq(name, testProcesses[i].name))
      return sys_create_process(testProcesses[i].entry, name, argc, argv, 0);

  return -1;
}

int64_t my_nice(uint64_t pid, uint64_t newPrio) {
  return sys_nice(pid, newPrio);
}

int64_t my_kill(uint64_t pid) {
  return sys_kill(pid);
}

int64_t my_block(uint64_t pid) {
  return sys_block(pid);
}

int64_t my_unblock(uint64_t pid) {
  return sys_unblock(pid);
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
  sys_yield();
  return 0;
}

int64_t my_wait(int64_t pid) {
  return sys_waitpid(pid);
}
