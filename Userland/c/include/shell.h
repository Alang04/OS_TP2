#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

#define CURSOR '_'
#define STDIN  0
#define STDOUT 1
#define WELCOME "Bienvenido al MASS OS!\n"
#define BUFF_LENGTH 100

typedef void (*Runnable)(void);

#define MAX_ARGS 10

typedef struct Command{
     char* name;
     Runnable function;
} Command;

typedef int64_t (*TestMain)(uint64_t argc, char *argv[]);

// Tests de la catedra: reciben los argumentos tipeados luego del nombre.
// 'ready' en 0 marca los que aun dependen de syscalls no implementadas.
typedef struct TestCommand{
     char* name;
     TestMain function;
     char* usage;
     uint8_t ready;
} TestCommand;

void shellPrintString(char *str);
void shellPutchar(char c, uint64_t fd);
void shellNewline(void);
void shellReadLine(char * buffer, uint64_t max);

#endif