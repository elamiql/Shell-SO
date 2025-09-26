#include "input.h"
#include "history.h"
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static struct termios orig_termios; //guarda configuracion inicial de la terminal 

//restaura la terminal a su estado original 
void disable_raw_mode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

//habilita modo raw para lectura caracter a caracter
void enable_raw_mode(){
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode); //asegurarse de restaurar al salir

    struct termios raw = orig_termios;

    raw.c_lflag &= ~(ECHO | ICANON); // desactivar eco y modo canon
    raw.c_cc[VMIN] = 1; // leer minimo un caracter
    raw.c_cc[VTIME] = 0; // sin timeout

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

//lee la entrada del usuario caracter por caracter
void read_input(char* buffer){
    int len = 0;
    buffer[len] = '\0';

    enable_raw_mode();

    char c;

    while (read(STDIN_FILENO, &c, 1) == 1){
        if (c == '\n'){ // enter
            buffer[len] = '\0';
            printf("\n");
            break;
        }
        else if (c == 127 || c == '\b'){ //backspace
            if (len > 0){
                len--;
                buffer[len] = '\0';
                printf("\b \b"); //borra caracter en pantalla
                fflush(stdout);
            }
        }
        else if (c == 27){ // secuencias de escape (flechas)
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1){
                if (seq[0] == '['){
                    if (seq[1] == 'A'){ //flecha arriba
                        const char* prevCmd = get_history_up();
                        if (prevCmd){
                            for (int i = 0; i < len; i++) printf("\b \b");
                            len = snprintf(buffer, MAX_INPUT, "%s", prevCmd);
                            printf("%s", buffer);
                            fflush(stdout);
                        }
                    } 
                    else if (seq[1] == 'B'){ //flecha abajo
                        const char* nextCmd = get_history_down();
                            for (int i = 0; i < len; i++) printf("\b \b");
                            if (nextCmd){
                                len = snprintf(buffer, MAX_INPUT, "%s", nextCmd);
                                printf("%s", buffer);
                                fflush(stdout);                          
                            }
                            else{
                                len = 0;
                                buffer[len] = '\0';
                                fflush(stdout);
                            }
                    }
                }

            }
        }
        else{ // caracter normal 
            if (len < MAX_INPUT - 1){
                buffer[len++] = c;
                write(STDOUT_FILENO, &c, 1);
            }
        }
    }

    disable_raw_mode(); // restaurar terminal 
}

// imprime el prompt personalizado con colores y directorio
void print_prompt(){
    char *cwd = malloc(1024);
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    if (getcwd(cwd, 1024) != NULL){
        printf("\033[1;32m%s@%s\033[0m:\033[1;36m%s\033[0m$ SCHELL: ",
               getenv("USER"),hostname, cwd);
    }
    else{
        perror("getcwd");
    }

    free(cwd);
    fflush(stdout);
}
