#ifndef MINISHELL_H
#define MINISHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define BUILTIN     1
#define EXTERNAL    2
#define NO_COMMAND  3

#define INPUT_SIZE  1024
#define MAX_ARGS    100
#define MAX_JOBS    100
#define MAX_COMPLETED_JOBS 100

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

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

extern volatile sig_atomic_t child_event_count;
extern pid_t completed_pids[MAX_COMPLETED_JOBS];
extern int completed_status[MAX_COMPLETED_JOBS];

int check_command_type(char *command);
int execute_builtin(char **args);
void execute_external(char **args);
void handle_prompt_assignment(char *input);
void signal_handler(int sig_num);
void add_job(pid_t pid, job_state state, const char *command);
int find_job_by_pid(pid_t pid);
void remove_job(int index);
void update_job_state(pid_t pid, job_state state);
void process_child_events(void);
int execute_jobs(void);
int execute_bg(char **args);
int execute_fg(char **args);

#endif