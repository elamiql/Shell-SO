#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64

int main() {
    
    char input[MAX_INPUT];
    char* args[MAX_ARGS];

    while(1){
        printf("tsh$ ");
        fflush(stdout);

        if (fgets(input, MAX_INPUT, stdin) == NULL){
            perror("fgets failed");
            continue;
        }
        
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0){
            printf("Gracias por usar SCHELL\n");
            break;
        }

        char* token = strtok(input, " ");

        int i = 0;

        while(token != NULL && i<=MAX_ARGS){
            args[i] = token;
            token = strtok(NULL, " ");
        }



    }

}
