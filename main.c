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

int main() {
    
    char input[MAX_INPUT];
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

        if (n>0){
            execPipes(cmds, n);
            free_pipe_cmds(cmds, n);
            continue;
        }

        char** args = parse_command(input);
        if (args[0] == NULL) {
            free_args(args);
            continue;
        }

        if (strcmp(args[0], "cd") == 0){
            if (args[1] == NULL){
                perror("cd: Faltan argumentos");
                free_args(args);
            }
            else if(chdir(args[1]) != 0){
                perror("cd failed");
                free_args(args);
            }
            free_args(args);
            continue;
        }

        pid_t pid = fork();
        
        if (pid == 0){
            execvp(args[0], args);
            fprintf(stderr, "%s: comando no encontrado\n", args[0]);
            free_args(args);
            exit(EXIT_FAILURE);
        }else if(pid > 0){
            int status;
            waitpid(pid, &status, 0);
            printf("Exist status: %d\n", status);

        }else{
            perror("fork failed");
        }

        free_args(args);
    }
}
