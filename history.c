#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_HISTORY 100             //maximo de comandos en el historial 
#define MAX_COMMAND_LENGHT 1024     //longitud maxima de cada comando

static char history[MAX_HISTORY][MAX_COMMAND_LENGHT]; //arreglo para almacenar comandos
static int history_count = 0;   //cantidad de comandos guardados
static int current_index = 0;   //indice actual para navegacion con flechas

//inicializa el historial
void init_history(){
    history_count = 0;
    current_index = 0;
}

//agrega un comando al historial
void add_to_history(char const *cmd){
    if (history_count < MAX_HISTORY){
        strncpy(history[history_count], cmd, MAX_COMMAND_LENGHT - 1);
        history[history_count][MAX_COMMAND_LENGHT] = '\0'; //asegurar terminacion nula
        history_count++;
    }
    current_index = history_count; //reiniciar indice de navegacion
}

//obtiene el comando anterior (flecha arriba)
const char* get_history_up(){
    if (current_index > 0){
        current_index--;
        return history[current_index];
    }
    return NULL;
}

//obtiene el siguiente comando (flecha abajo)
const char* get_history_down(){
    if (current_index < history_count - 1){
        current_index++;
        return history[current_index];
    }
    else{
        current_index = history_count;
        return ""; // si no hay siguiente, devuelve cadena vacia
    }

    return NULL;
}

// imprime todo el historial de comandos
void print_history(){
    for (int i=0; i<history_count; i++){
        printf("%s\n", history[i]);
    }
}
