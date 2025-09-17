#ifndef PIPES_H
#define PIPES_H

#define MAX_PIPE_CMDS 16
#define MAX_TOKENS 128

int is_special_char(char c);
int parsePipe(char* input, char** cmds, int max_cmds);
char** parse_command(char* line);
void free_args(char** args);
void free_pipe_cmds(char** cmds, int n);
void execPipes(char** cmds, int n);

#endif