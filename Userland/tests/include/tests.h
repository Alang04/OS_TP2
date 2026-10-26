#ifndef TESTS_H
#define TESTS_H

#include <stdint.h>

// Tests provistos por la catedra (https://github.com/alejoaquili/ITBA-72.11-SO)
// Todos devuelven -1 si los parametros son invalidos.
int64_t test_mm(uint64_t argc, char *argv[]);        // test_mm <max_memory>
int64_t test_processes(uint64_t argc, char *argv[]); // test_proc <max_processes>
int64_t test_prio(uint64_t argc, char *argv[]);      // test_prio <max_value>
int64_t test_sync(uint64_t argc, char *argv[]);      // test_sync <n> <use_sem>

// Procesos auxiliares que crean los tests
void zero_to_max(void);
int64_t my_process_inc(uint64_t argc, char *argv[]);

#endif
