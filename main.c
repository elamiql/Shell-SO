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

pid_t child_pid = -1; // PID del proceso hijo actual 

//Manejador de señal para alarmas (termina el proceso si excede el tiempo)
void alarma(int sig){
    if (child_pid > 0){
        kill(child_pid, SIGKILL);
        printf("Proceso excedió el tiempo máximo y fue terminado.\n");
    }
}

int main(){
    char input[MAX_INPUT];      //buffer para la entrada de usuario
    char *cmds[MAX_PIPE_CMDS];  //comandos separados por pipes

    while (1){
        print_prompt();         //muestra el prompt
        read_input(input);      //lee la entrada del usuario (modo raw)

        if (strlen(input) == 0) //ignora entradas vacias
            continue;

        input[strcspn(input, "\n")] = '\0'; // elimina salto de linea
        add_to_history(input);              //guarda en el historial

        //comando interno para salir
        if (strcmp(input, "exit") == 0){
            printf("Gracias por usar SCHELL\n");
            break;
        }

        //comando interno para printear el historial
        if (strcmp(input, "print_history") == 0){
            print_history();
            continue;
        }

        //verifica si hay pipes en la entrada
        int n = parsePipe(input, cmds, 16);

        if (n > 0){
            execPipes(cmds, n);             //ejecutar pipeline
            free_pipe_cmds(cmds, n);        //liberar la memoria
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

        //comando interno miprof (monitoreo de procesos)
        if (strcmp(args[0], "miprof") == 0){
            if (args[1] == NULL){
                fprintf(stderr, "Uso: miprof [ejec|ejecsave archivo|ejecutar maxtiempo] comando args...\n");
                free_args(args);
                continue;
            }

            int save = 0;        //flag para guardar la salida del archivo
            char *filename = NULL;
            int cmd_index = 2; // índice del comando a ejecutar

            // miprof ejec: ejecuta y muestra resultados
            if (strcmp(args[1], "ejec") == 0){
                save = 0;
                cmd_index = 2;
            }
            // miprof ejecsave archivo: guarda salida del archivo
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
            // miprof ejecutar tiempo comando: ejecuta con límite de tiempo
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
                    execvp(args[cmd_index], &args[cmd_index]); // ejecutar comando
                    perror("execvp falló");
                    exit(EXIT_FAILURE);
                }
                else if (child_pid > 0){
                    signal(SIGALRM, alarma); //activar alarma
                    alarm(maxtiempo);

                    int status;
                    wait4(child_pid, &status, 0, &usage); //esperar proceso

                    clock_gettime(CLOCK_MONOTONIC, &end);

                    double real_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

                    //reporte de recursos
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

                    alarm(0); //desactivar alarma
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

            // ejecucion normal de miprof (ejec o ejecsave)
            pid_t pid = fork();
            if (pid == 0){
                if (save){
                    FILE *f = fopen(filename, "a");
                    if (!f){
                        perror("No se pudo abrir el archivo");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fileno(f), STDOUT_FILENO); // redirigir stdout
                    dup2(fileno(f), STDERR_FILENO); // redirigir stderr
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

        // ejecucion de comandos internos
        pid_t pid = fork();

        if (pid == 0){
            execvp(args[0], args); // ejecutar comando
            fprintf(stderr, "%s: comando no encontrado\n", args[0]);
            free_args(args);
            exit(EXIT_FAILURE);
        }
        else if (pid > 0){
            int status;
            waitpid(pid, &status, 0); // esperar al hijo
            printf("Exit status: %d\n", status);
        }
        else{
            perror("fork failed");
        }

        free_args(args); //liberar memoria
    }
}
