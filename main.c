#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include <ctype.h>
#include <sys/resource.h>
#include <signal.h>
#include <time.h>
#include "history.h"
#include "input.h"
#include "pipes.h"

pid_t child_pid = -1;

void alarma(int sig){
    if (child_pid > 0){
        kill(child_pid, SIGKILL);
        printf("Proceso excedió el tiempo máximo y fue terminado.\n");
    }
}

int main(){
    char input[MAX_INPUT];
    char *cmds[MAX_PIPE_CMDS];

    while (1){
        print_prompt();
        read_input(input);

        if (strlen(input) == 0)
            continue;

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

        if (n > 0){
            execPipes(cmds, n);
            free_pipe_cmds(cmds, n);
            continue;
        }

        char **args = parse_command(input);
        if (args[0] == NULL){
            free_args(args);
            continue;
        }

        // Comando interno cd
        if (strcmp(args[0], "cd") == 0){
            if (args[1] == NULL){
                perror("cd: Faltan argumentos");
            }
            else if (chdir(args[1]) != 0){
                perror("cd failed");
            }
            free_args(args);
            continue;
        }

        if (strcmp(args[0], "miprof") == 0){
            if (args[1] == NULL){
                fprintf(stderr, "Uso: miprof [ejec|ejecsave archivo|ejecutar maxtiempo] comando args...\n");
                free_args(args);
                continue;
            }

            int save = 0;
            char *filename = NULL;
            int cmd_index = 2; // índice del comando a ejecutar

            if (strcmp(args[1], "ejec") == 0){
                save = 0;
                cmd_index = 2;
            }
            else if (strcmp(args[1], "ejecsave") == 0){
                if (args[2] == NULL){
                    fprintf(stderr, "Uso: miprof ejecsave archivo comando args...\n");
                    free_args(args);
                    continue;
                }
                save = 1;
                filename = args[2];
                cmd_index = 3;
            }
            else if (strcmp(args[1], "ejecutar") == 0){
                if (args[2] == NULL || args[3] == NULL){
                    fprintf(stderr, "Uso: miprof ejecutar maxtiempo comando args...\n");
                    free_args(args);
                    continue;
                }

                int maxtiempo = atoi(args[2]);
                if (maxtiempo <= 0){
                    fprintf(stderr, "El tiempo máximo debe ser mayor a 0.\n");
                    free_args(args);
                    continue;
                }

                int cmd_index = 3;
                struct rusage usage;
                struct timespec start, end;
                clock_gettime(CLOCK_MONOTONIC, &start);

                child_pid = fork();
                if (child_pid == 0){
                    execvp(args[cmd_index], &args[cmd_index]);
                    perror("execvp falló");
                    exit(EXIT_FAILURE);
                }
                else if (child_pid > 0){
                    signal(SIGALRM, alarma);
                    alarm(maxtiempo);

                    int status;
                    wait4(child_pid, &status, 0, &usage);

                    clock_gettime(CLOCK_MONOTONIC, &end);

                    double real_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

                    printf("\n--- Resultados miprof ejecutar ---\n");
                    if (WIFEXITED(status)){
                        printf("Exit code: %d\n", WEXITSTATUS(status));
                    }
                    else if (WIFSIGNALED(status)){
                        printf("Terminado por señal %d\n", WTERMSIG(status));
                    }

                    printf("Tiempo real: %.3f s\n", real_time);
                    printf("Tiempo usuario: %.3f s\n",
                           usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6);
                    printf("Tiempo sistema: %.3f s\n",
                           usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6);
                    printf("Memoria máxima residente: %ld KB\n", usage.ru_maxrss);

                    alarm(0);
                }
                else{
                    perror("fork falló");
                }

                free_args(args);
                continue;
            }

            else{
                fprintf(stderr, "Opción inválida: %s\n", args[1]);
                free_args(args);
                continue;
            }

            pid_t pid = fork();
            if (pid == 0){
                if (save){
                    FILE *f = fopen(filename, "a");
                    if (!f){
                        perror("No se pudo abrir el archivo");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fileno(f), STDOUT_FILENO);
                    dup2(fileno(f), STDERR_FILENO);
                    fclose(f);
                }
                execvp(args[cmd_index], &args[cmd_index]);
                perror("execvp falló");
                exit(EXIT_FAILURE);
            }
            else if (pid > 0){
                int status;
                waitpid(pid, &status, 0);
                printf("Exit status: %d\n", status);
            }
            else{
                perror("fork falló");
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
        }
        else if (pid > 0){
            int status;
            waitpid(pid, &status, 0);
            printf("Exit status: %d\n", status);
        }
        else{
            perror("fork failed");
        }

        free_args(args);
    }
}
