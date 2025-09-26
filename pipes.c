#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include <ctype.h>
#include "history.h"
#include "input.h"
#include "pipes.h"

// verifica si un caracter es especial (pipes o redirecciones)
int is_special_char(char c){
    return c == '|' || c == '>' || c == '<';
}

//divide la entrada en comandos separados por pipes
int parsePipe(char* input, char** cmds, int max_cmds){

    if (strchr(input, '|') == NULL){
        return 0;
    }

    int n = 0;
    char* cmd;
    char* input_copy = strdup(input);
    char* input_ptr = input_copy;

    while ((cmd = strsep(&input_copy, "|")) != NULL && n < max_cmds){
        while (*cmd == ' ') cmd++; //eliminar espacios iniciales
        char* end = cmd + strlen(cmd) - 1;
        while (end > cmd && isspace(*end)) end--; //eliminar espacios finales
        *(end + 1) = '\0';
        cmds[n++] = strdup(cmd); //guardar comando
    }
    free(input_ptr);
    return n; //cantidad de comandos
}

//separa un comando en argumentos
char** parse_command(char* line){
    char** argv = malloc(sizeof(char*) * MAX_TOKENS);
    int argc = 0;
    int i = 0;
    int len = strlen(line);

    while (i < len){
        while (isspace(line[i])) i++; //saltar espacios
        if (i>len) break;

        char token[1024];
        int pos = 0;

        //manejar comillas
        if (line[i] == '\'' || line[i] == '"'){
            char quote = line[i++];
            while (i<len && line[i] != quote){
                token[pos++] = line[i++];
            }
            if (i<len) i++;
        }
        else if(is_special_char(line[i])){ //token especial 
            token[pos++] = line[i++];
        }
        else{ //token normal
            while (i<len && !isspace(line[i]) && !is_special_char(line[i])){
                token[pos++] = line[i++];
            }
        }

        token[pos] = '\0';
        argv[argc++] = strdup(token);

        if (argc >= MAX_TOKENS - 1) break;
    }
    argv[argc] = NULL;
    return argv;
}

// ejecuta una serie de comandos conectados por pipes
void execPipes(char** cmds, int n){
    int i;
    int in_fd = 0;
    pid_t pid;
    pid_t pids[MAX_PIPE_CMDS];
    
    int pipefd[2];
    
    for (i = 0; i < n; i++){
        char** args = parse_command(cmds[i]);
        
        if (args[0] == NULL){
            free_args(args);
            continue;
        }
        
        if (i<n-1) { // crear pipe si no es el ultimo comando
            if (pipe(pipefd) == -1){
                perror("pipe failed");
                free_args(args);
                return;
            }
        }
        
        pid = fork();
        pids[i] = pid;
        
        if (pid == 0){ // hijo
            if (in_fd != 0){
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            
            if (i<n-1){ // si no es el ultimo, redirigir salida al pipe
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            execvp(args[0], args); //ejecutar comando
            fprintf(stderr, "%s: comando no encontrado\n", args[0]);
            free_args(args);
            exit(EXIT_FAILURE);
        }
        else if (pid > 0){ //padre
            if (in_fd != 0) close(in_fd);
            if (i < n-1){
                close(pipefd[1]);
                in_fd = pipefd[0]; //entrada al siguiente comando
            }
        }
        else{
            perror("fork failed");
            free_args(args);
            return;
        }
        
        free_args(args);
    }
    //esperar a todos los hijos
    for (int i = 0; i<n; i++){
        if (pids[i] > 0){
            int status;
            waitpid(pids[i], &status, 0);
        }
    }
}

//libera memoria de los argumentos
void free_args(char** args){
    if (args){
        for (int i=0; args[i]; i++){
            free(args[i]);
        }
        free(args);
    }
}

//libera memoria de los comandos de pipe
void free_pipe_cmds(char** cmds, int n){
    for (int i=0; i<n; i++){
        if (cmds[i]){
            free(cmds[i]);
        }
    }
}