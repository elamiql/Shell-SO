#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include "history.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

static struct termios orig_termios;

void disable_raw_mode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode(){
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;

    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void read_input(char* buffer){
    int len = 0;
    buffer[len] = '\0';

    enable_raw_mode();

    char c;

    while (read(STDIN_FILENO, &c, 1) == 1){
        if (c == '\n'){
            buffer[len] = '\0';
            printf("\n");
            break;
        }else if (c == 127 || c == '\b'){
            if (len > 0){
                len--;
                buffer[len] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
        }else if (c == 27){
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1)){
                if (seq[0] == '['){
                    if (seq[1] == 'A'){
                        const char* prevCmd = get_history_up();
                        if (prevCmd){
                            for (int i = 0; i < len; i++) printf("\b \b");
                            len = snprintf(buffer, MAX_INPUT, "%s", prevCmd);
                            printf("%s", buffer);
                            fflush(stdout);
                        }
                    } else if (seq[1] == 'B'){
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
        else{
            if (len < MAX_INPUT - 1){
                buffer[len++] = c;
                write(STDOUT_FILENO, &c, 1);
            }
        }
    }

    disable_raw_mode();
}
int main() {
    
    char input[MAX_INPUT];
    char* args[MAX_ARGS];

    while(1){
        printf("tsh$ ");
        fflush(stdout);

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
            perror("excecvp failed");
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
