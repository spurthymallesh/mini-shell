#ifndef MINISHELL_H
#define MINISHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define INPUT_SIZE 1024
#define MAX_ARGS 64

#define BUILTIN 1
#define EXTERNAL 2
#define NO_COMMAND 3

#define MAX_JOBS 100

typedef enum
{
    JOB_RUNNING,
    JOB_STOPPED
} job_state;

typedef struct
{
    int job_id;
    pid_t pid;
    job_state state;
    char command[INPUT_SIZE];
} job_t;

extern job_t jobs[MAX_JOBS];
extern int job_count;

extern int last_exit_status;
extern char prompt[INPUT_SIZE];
extern pid_t foreground_pid;

int check_command_type(char *command);
int execute_builtin(char **args);
void execute_external(char **args);
void handle_prompt_assignment(char *input);
void signal_handler(int sig_num);

#endif