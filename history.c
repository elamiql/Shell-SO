#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_HISTORY 100
#define MAX_COMMAND_LENGHT 1024

static char history[MAX_HISTORY][MAX_COMMAND_LENGHT];
static int history_count = 0;
static int current_index = 0;

void init_history(){
    history_count = 0;
    current_index = 0;
}

void add_to_history(char const *cmd){
    if (history_count < MAX_HISTORY){
        strncpy(history[history_count], cmd, MAX_COMMAND_LENGHT - 1);
        history[history_count][MAX_COMMAND_LENGHT] = '\0';
        history_count++;
    }
    current_index = history_count;
}

const char* get_history_up(){
    if (current_index > 0){
        current_index--;
        return history[current_index];
    }
    return NULL;
}

const char* get_history_down(){
    if (current_index < history_count - 1){
        current_index++;
        return history[current_index];
    }
    else{
        current_index = history_count;
        return "";
    }

    return NULL;
}

void print_history(){
    for (int i=0; i<history_count; i++){
        printf("%s\n", history[i]);
    }
}
