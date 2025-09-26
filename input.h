#ifndef INPUT_H
#define INPUT_H

#define MAX_INPUT 1024
#define MAX_ARGS 64

void read_input(char* buffer);
void print_prompt(void);
void enable_raw_mode(void);
void disable_raw_mode(void);

#endif