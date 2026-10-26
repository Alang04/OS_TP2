// Mini libc para userland: memoria dinamica via syscalls y printf basico
// (memset esta definido en _loader.c)
#include <stdarg.h>
#include "userlib.h"

void * malloc(size_t size){
    return sys_malloc(size);
}

void free(void * ptr){
    sys_free(ptr);
}

static int printStr(const char * s){
    int n = 0;
    if(s == 0){
        s = "(null)";
    }
    while(s[n] != '\0'){
        putchar(s[n++]);
    }
    return n;
}

// printf minimo: soporta %d %i %u %x %s %c y %%
int printf(const char * fmt, ...){
    va_list args;
    va_start(args, fmt);
    char num[24];
    int written = 0;

    for(const char * p = fmt; *p != '\0'; p++){
        if(*p != '%'){
            putchar(*p);
            written++;
            continue;
        }

        p++;
        switch(*p){
            case 'd':
            case 'i': {
                int value = va_arg(args, int);
                if(value < 0){
                    putchar('-');
                    written++;
                    num_to_str((uint64_t)(-(int64_t)value), num, 10);
                } else {
                    num_to_str((uint64_t)value, num, 10);
                }
                written += printStr(num);
                break;
            }
            case 'u':
                num_to_str(va_arg(args, unsigned int), num, 10);
                written += printStr(num);
                break;
            case 'x':
                num_to_str(va_arg(args, unsigned int), num, 16);
                written += printStr(num);
                break;
            case 's':
                written += printStr(va_arg(args, char *));
                break;
            case 'c':
                putchar((char)va_arg(args, int));
                written++;
                break;
            case '%':
                putchar('%');
                written++;
                break;
            case '\0':
                p--; // '%' al final del formato: se ignora
                break;
            default:
                putchar('%');
                putchar(*p);
                written += 2;
                break;
        }
    }

    va_end(args);
    return written;
}
