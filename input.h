#ifndef INPUT_H
#define INPUT_H

#define MAX_INPUT 1024
#define MAX_ARGS 64

void enable_raw_mode();
void disable_raw_mode();

void read_input(char* buffer);

void print_prompt();

#endif