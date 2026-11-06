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

typedef int64_t (*CommandMain)(uint64_t argc, char *argv[]);

// Comandos que corren como un proceso propio (apps y tests de la catedra):
// reciben los argumentos tipeados luego del nombre.
typedef struct ProcessCommand{
     char* name;
     CommandMain function;
     char* usage;
} ProcessCommand;

void shellPrintString(char *str);
void shellPutchar(char c, uint64_t fd);
void shellNewline(void);
void shellReadLine(char * buffer, uint64_t max);

#endif