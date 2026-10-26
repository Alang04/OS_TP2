#include <stdint.h>
#include "userlib.h"
#include "syscall.h"
#include "test_util.h"
#include "tests.h"

#define TOTAL_PROCESSES 3

#define LOWEST 0  // TODO: Change as required
#define MEDIUM 1  // TODO: Change as required
#define HIGHEST 2 // TODO: Change as required

static int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};

static uint64_t max_value = 0;

void zero_to_max(void) {
  uint64_t value = 0;

  while (value++ != max_value);

  printf("PROCESS %d DONE!\n", (int)my_getpid());
}

int64_t test_prio(uint64_t argc, char *argv[]) {
  int64_t pids[TOTAL_PROCESSES];
  char *ztm_argv[] = {0};
  uint64_t i;

  if (argc != 1)
    return -1;

  if ((int64_t)(max_value = satoi(argv[0])) <= 0)
    return -1;

  printf("SAME PRIORITY...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = my_create_process("zero_to_max", 0, ztm_argv);

  // Expect to see them finish at the same time

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_wait(pids[i]);

  printf("SAME PRIORITY, THEN CHANGE IT...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++) {
    pids[i] = my_create_process("zero_to_max", 0, ztm_argv);
    my_nice(pids[i], prio[i]);
    printf("  PROCESS %d NEW PRIORITY: %d\n", (int)pids[i], (int)prio[i]);
  }

  // Expect the priorities to take effect

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_wait(pids[i]);

  printf("SAME PRIORITY, THEN CHANGE IT WHILE BLOCKED...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++) {
    pids[i] = my_create_process("zero_to_max", 0, ztm_argv);
    my_block(pids[i]);
    my_nice(pids[i], prio[i]);
    printf("  PROCESS %d NEW PRIORITY: %d\n", (int)pids[i], (int)prio[i]);
  }

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_unblock(pids[i]);

  // Expect the priorities to take effect

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_wait(pids[i]);

  return 0;
}
