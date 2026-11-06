#include "shell.h"
#include "userlib.h"

static int cursorVisible = 0;
static void showCursor(void){
    if(!cursorVisible){
        char cursor = CURSOR;
        sys_write(STDOUT, &cursor, 1);
        cursorVisible = 1;
    }
}

static void hideCursor(void){
    if(cursorVisible){
        char backspace = '\b';
        sys_write(STDOUT, &backspace, 1);
        cursorVisible = 0;
    }
}

// Libera los hijos en background que ya terminaron y avisa cuales fueron.
// Sin señales no hay SIGCHLD, asi que se revisa antes de cada prompt
static void reapBackground(void){
    int64_t pid;
    while((pid = sys_wait_nohang()) != -1){
        printf("[%d] termino\n", (int)pid);
    }
}

// Bucle principal de la shell de usuario
int main(void){
    shellPrintString(WELCOME);
    shellNewline();
    shellPrintString("Escriba 'help' para listar comandos.\n");

    char buff[BUFF_LENGTH];
    while(1){
        reapBackground();
        shellPrintString("> ");
        showCursor();
        shellReadLine(buff, BUFF_LENGTH);
        hideCursor();
        shellNewline();
        processLine(buff, 0);
    }

    return 0;
}

// Lee una línea desde teclado con cursor parpadeante.
// Espera bloqueado hasta que llegue una tecla o pase BLINK_TICKS: en ese caso
// solo alterna el cursor y vuelve a esperar (no hay busy waiting)
void shellReadLine(char * buffer, uint64_t max){
    char c;
    uint32_t idx = 0;

    // Asegurar que el cursor esté visible al iniciar la lectura
    showCursor();

    while(1){
        if(sys_read_timeout(&c, 1, BLINK_TICKS) == 1){
            if(c == '\n'){
                break;
            }

            if(c == '\b'){
                if(idx > 0){
                    idx--;
                    hideCursor();               // remover cursor '_'
                    shellPutchar('\b', STDOUT); // borrar último carácter
                    showCursor();               // volver a dibujar cursor
                }
            } else if(c == '+'){
                // Aumentar fuente y redibujar contenido
                hideCursor();
                sys_increase_fontsize();
                redrawFont();
                showCursor();
            } else if(c == '-'){
                // Disminuir fuente y redibujar contenido
                hideCursor();
                sys_decrease_fontsize();
                redrawFont();
                showCursor();
            } else {
                if(idx + 1 < max){ // dejar lugar para terminador NUL
                    buffer[idx++] = c;
                    hideCursor();
                    shellPutchar(c, STDOUT);
                    showCursor();
                }
            }
        } else if(cursorVisible){
            // Se vencio la espera sin input: parpadeo del cursor
            hideCursor();
        } else {
            showCursor();
        }
    }

    buffer[idx] = 0;
}

// Imprime una cadena en STDOUT
void shellPrintString(char *str){
    if(str == 0){
        return;
    }
    for(uint32_t i = 0; str[i] != '\0'; i++){
        shellPutchar(str[i], STDOUT);
    }
}

// Escribe un caracter en el descriptor indicado
void shellPutchar(char c, uint64_t fd){
    // Registrar en redraw buffer antes de imprimir
    redraw_append_char(c, fd);
    sys_write(fd, &c, 1); // escribo el caracter
}

// Salto de línea
void shellNewline(){
    shellPutchar('\n', STDOUT);
}