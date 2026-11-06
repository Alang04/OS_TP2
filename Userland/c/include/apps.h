#ifndef APPS_H
#define APPS_H

#include <stdint.h>

// Aplicaciones de userland pedidas por la consigna. Cada una corre como un proceso
// propio: reciben los argumentos tipeados luego del nombre y devuelven -1 si los
// parametros son invalidos (la shell muestra el uso).
int64_t mem(uint64_t argc, char ** argv);     // mem
int64_t ps(uint64_t argc, char ** argv);      // ps
int64_t loop(uint64_t argc, char ** argv);    // loop [segundos]
int64_t kill(uint64_t argc, char ** argv);    // kill <pid>
int64_t nice(uint64_t argc, char ** argv);    // nice <pid> <prioridad>
int64_t block(uint64_t argc, char ** argv);   // block <pid>

#endif
