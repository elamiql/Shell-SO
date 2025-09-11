#ifndef HISTORY_H
#define HISTORY_H

#define MAX_HISTORY 150
#define COMMAND_LENGHT 1024


void add_to_history(const char *cmd);
const char* get_history_up();
const char* get_history_down();
void print_history();

#endif