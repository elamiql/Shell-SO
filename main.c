#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include "history.h"
#include "input.h"

#define MAX_PIPE_CMDS 16

int parsePipe(char* input, char** cmds, int max_cmds){
    int n = 0;
    char* cmd;

    while ((cmd = strsep(&input, "|")) != NULL && n < max_cmds){
        while (*cmd == ' ') cmd++;
        cmds[n++] = cmd;
    }
    return n;
}

int parseArgs(char* cmd, char** args){
    int i = 0;
    char* token = strtok(cmd, " ");
    while (token != NULL && i < MAX_ARGS - 1){
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    return i;
}

void execPipes(char** cmds, int n){
    int i;
    int in_fd = 0;
    pid_t pid;

    int pipefd[2];

    for (i = 0; i < n; i++){
        char* args[MAX_ARGS];
        parseArgs(cmds[i], args);

        if (i<n-1) pipe(pipefd);

        pid = fork();

        if (pid == 0){
            if (in_fd != 0){
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if (i<n-1){
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            execvp(args[0], args);
            fprintf(stderr, "%s: comando no encontrado\n", args[0]);
            exit(EXIT_FAILURE);
        }
        else{
            if (in_fd != 0) close(in_fd);
            if (i<n-1){
                close(pipefd[1]);
                in_fd = pipefd[0];
            }
        }
    }

    for (i = 0; i < n; i++) wait(NULL);
}








int main() {
    
    char input[MAX_INPUT];
    char* args[MAX_ARGS];
    char* cmds[MAX_PIPE_CMDS];

    while(1){
        print_prompt();
        read_input(input);

        if (strlen(input) == 0) continue;

        input[strcspn(input, "\n")] = '\0';
        add_to_history(input);

        if (strcmp(input, "exit") == 0){
            printf("Gracias por usar SCHELL\n");
            break;
        }

        if (strcmp(input, "print_history") == 0){
            print_history();
            continue;
        }

        int n = parsePipe(input, cmds, 16);

        if (n>1){
            execPipes(cmds, n);
            continue;
        }

        char* token = strtok(input, " ");

        int i = 0;

        while(token != NULL && i < MAX_ARGS){
            args[i++] = token;
            token = strtok(NULL, " ");
        }

        args[i] = NULL;

        if (strcmp(args[0], "cd") == 0){
            if (args[1] == NULL){
                perror("cd: Faltan argumentos");
            }
            else if(chdir(args[1]) != 0){
                perror("cd failed");
            }
            continue;
        }

        pid_t pid = fork();
        
        if (pid == 0){
            execvp(args[0], args);
            fprintf(stderr, "%s: comando no encontrado\n", args[0]);
            exit(EXIT_FAILURE);
        }else if(pid > 0){
            int status;
            waitpid(pid, &status, 0);
            printf("Exist status: %d\n", status);

        }else{
            perror("fork failed");
        }

    }

}
