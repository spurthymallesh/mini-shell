#include "minishell.h"
#include <errno.h>

int last_exit_status = 0;
char prompt[INPUT_SIZE] = "msh> ";
pid_t foreground_pid = -1;

job_t jobs[MAX_JOBS];
int job_count = 0;

volatile sig_atomic_t child_event_count = 0;
pid_t completed_pids[MAX_COMPLETED_JOBS];
int completed_status[MAX_COMPLETED_JOBS];

static void install_signal_handlers(void);
static int tokenize_input(char *input, char **args);


int main(void)
{
    char input[INPUT_SIZE];
    char original_command[INPUT_SIZE];
    char *args[MAX_ARGS];

    install_signal_handlers();

    while (1)
    {
    
        process_child_events();

    
        printf("%s", prompt);
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            /*
             * Signal interrupted fgets().
             * Re-display prompt.
             */
            if (errno == EINTR)
            {
                clearerr(stdin);
                process_child_events();
                continue;
            }

            /*
             * EOF / Ctrl+D.
             */
            printf("\n");
            break;
        }

        /*
         * Remove trailing newline.
         */
        input[strcspn(input, "\n")] = '\0';

        /*
         * Ignore empty commands.
         */
        if (input[0] == '\0')
        {
            continue;
        }

        /*
         * Handle PS1=NEW_PROMPT.
         *
         * Only PS1= without spaces is treated as
         * prompt assignment.
         */
        if (strncmp(input, "PS1=", 4) == 0)
        {
            handle_prompt_assignment(input);
            last_exit_status = 0;
            continue;
        }

        /*
 * Check for pipe functionality.
 */
if (strchr(input, '|') != NULL)
{
    last_exit_status = execute_pipeline(input);
    continue;
}

        /*
         * Save original command before strtok()
         * modifies input.
         */
        snprintf(original_command,
                 sizeof(original_command),
                 "%s",
                 input);

        /*
         * Tokenize command line.
         */
        if (tokenize_input(input, args) == 0)
        {
            continue;
        }

        /*
         * Check whether command is running in background.
         *
         * Example:
         *
         * sleep 20 &
         *
         * becomes:
         *
         * args[0] = "sleep"
         * args[1] = "20"
         * args[2] = NULL
         */
        int background = 0;
        int last = 0;

        while (args[last] != NULL)
        {
            last++;
        }

        if (last > 0 && strcmp(args[last - 1], "&") == 0)
        {
            background = 1;
            args[last - 1] = NULL;
        }

        /*
         * Identify command type.
         */
        int command_type = check_command_type(args[0]);

        /*
         * -------------------------------------------------
         * BUILT-IN COMMAND
         * -------------------------------------------------
         */
        if (command_type == BUILTIN)
        {
            /*
             * At this stage built-ins are executed only
             * in the foreground.
             */
            if (background)
            {
                fprintf(stderr,
                        "msh: background execution of built-in "
                        "commands is not supported\n");

                last_exit_status = 1;
                continue;
            }

            last_exit_status = execute_builtin(args);
        }

        /*
         * -------------------------------------------------
         * EXTERNAL COMMAND
         * -------------------------------------------------
         */
        else if (command_type == EXTERNAL)
        {
            pid_t pid;
            int status;

            /*
             * Create child process.
             */
            pid = fork();

            if (pid < 0)
            {
                perror("fork");
                last_exit_status = 1;
                continue;
            }

            /*
             * -------------------------------------------------
             * CHILD PROCESS
             * -------------------------------------------------
             */
            if (pid == 0)
            {
                /*
                 * Child should receive normal terminal signals.
                 */
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGCHLD, SIG_DFL);

                /*
                 * Execute external command.
                 */
                execvp(args[0], args);

                /*
                 * execvp() failed.
                 */
                perror("execvp");
                exit(EXIT_FAILURE);
            }

            /*
             * -------------------------------------------------
             * BACKGROUND PROCESS
             * -------------------------------------------------
             */
            if (background)
            {
                /*
                 * Make sure there is room in job table.
                 */
                if (job_count >= MAX_JOBS)
                {
                    fprintf(stderr,
                            "msh: maximum number of jobs reached\n");

                    kill(pid, SIGTERM);

                    /*
                     * Reap child so that it does not become
                     * a zombie.
                     */
                    while (waitpid(pid, NULL, 0) < 0)
                    {
                        if (errno != EINTR)
                        {
                            break;
                        }
                    }

                    last_exit_status = 1;
                    continue;
                }

                /*
                 * Add background process to job table.
                 */
                add_job(pid, JOB_RUNNING, original_command);

                /*
                 * Print job number and PID.
                 */
                printf("[%d] %d\n",
                       jobs[job_count - 1].job_id,
                       pid);

                fflush(stdout);

                /*
                 * Background command was successfully started.
                 */
                last_exit_status = 0;

                continue;
            }

            /*
             * -------------------------------------------------
             * FOREGROUND PROCESS
             * -------------------------------------------------
             */

            foreground_pid = pid;

            while (1)
            {
                pid_t result;

                result = waitpid(pid, &status, WUNTRACED);

                if (result == pid)
                {
                    /*
                     * Child state has been received.
                     */
                    break;
                }

                if (result < 0 && errno == EINTR)
                {
                    /*
                     * Signal interrupted waitpid().
                     * Try again.
                     */
                    continue;
                }

                if (result < 0)
                {
                    perror("waitpid");
                    last_exit_status = 1;
                    break;
                }
            }

            /*
             * -------------------------------------------------
             * PROCESS EXIT STATUS
             * -------------------------------------------------
             */

            if (WIFEXITED(status))
            {
                /*
                 * Child exited normally.
                 */
                last_exit_status = WEXITSTATUS(status);
            }
            else if (WIFSIGNALED(status))
            {
                /*
                 * Child was terminated by a signal.
                 *
                 * Shell convention:
                 * exit status = 128 + signal number
                 */
                last_exit_status = 128 + WTERMSIG(status);
            }
            else if (WIFSTOPPED(status))
            {
                /*
                 * Child was stopped by Ctrl+Z.
                 *
                 * Add it to the job table so that
                 * bg/fg can use it later.
                 */
                if (job_count < MAX_JOBS)
                {
                    add_job(pid, JOB_STOPPED, original_command);

                    printf("\n[%d] Stopped\n",
                           jobs[job_count - 1].job_id);
                }
                else
                {
                    fprintf(stderr,
                            "\nmsh: maximum number of jobs reached\n");
                }

                fflush(stdout);
            }

            /*
             * No foreground process now.
             */
            foreground_pid = -1;
        }

        /*
         * If command was not recognized for some reason,
         * keep the shell alive.
         */
        else if (command_type == NO_COMMAND)
        {
            continue;
        }
    }

    return 0;
}


/*
 * ---------------------------------------------------------
 * INSTALL SIGNAL HANDLERS
 * ---------------------------------------------------------
 */
static void install_signal_handlers(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

    /*
     * Shell signal handler.
     */
    sa.sa_handler = signal_handler;

    sigemptyset(&sa.sa_mask);

    /*
     * Do NOT use SA_RESTART here.
     *
     * We want SIGINT/SIGTSTP/SIGCHLD to interrupt
     * fgets()/waitpid() so that the shell can react
     * immediately.
     */
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);

    /*
     * SIGCHLD handler.
     *
     * When a background child terminates, SIGCHLD
     * allows the shell to collect its exit status.
     */
    sigaction(SIGCHLD, &sa, NULL);
}


/*
 * ---------------------------------------------------------
 * TOKENIZE INPUT
 * ---------------------------------------------------------
 *
 * Example:
 *
 *     ls -l /tmp
 *
 * becomes:
 *
 *     args[0] = "ls"
 *     args[1] = "-l"
 *     args[2] = "/tmp"
 *     args[3] = NULL
 *
 */
static int tokenize_input(char *input, char **args)
{
    int count = 0;
    char *token;

    token = strtok(input, " \t");

    while (token != NULL && count < MAX_ARGS - 1)
    {
        args[count++] = token;

        token = strtok(NULL, " \t");
    }

    args[count] = NULL;

    return count;
}


/*
 * ---------------------------------------------------------
 * PROCESS COMPLETED BACKGROUND CHILDREN
 * ---------------------------------------------------------
 */
void process_child_events(void)
{
    sig_atomic_t count;
    int i;

    count = child_event_count;

    for (i = 0; i < count; i++)
    {
        pid_t pid;
        int status;
        int index;
        int exit_status;

        pid = completed_pids[i];
        status = completed_status[i];

        /*
         * Find child in job table.
         */
        index = find_job_by_pid(pid);

        if (index == -1)
        {
            continue;
        }

        /*
         * Child exited normally.
         */
        if (WIFEXITED(status))
        {
            exit_status = WEXITSTATUS(status);

            printf("\n[%d] Done %s (exit status: %d)\n",
                   jobs[index].job_id,
                   jobs[index].command,
                   exit_status);

            last_exit_status = exit_status;
        }

        /*
         * Child was terminated by a signal.
         */
        else if (WIFSIGNALED(status))
        {
            exit_status = 128 + WTERMSIG(status);

            printf("\n[%d] Terminated %s (exit status: %d)\n",
                   jobs[index].job_id,
                   jobs[index].command,
                   exit_status);

            last_exit_status = exit_status;
        }

        fflush(stdout);

        /*
         * Remove completed process from job table.
         */
        remove_job(index);
    }

    /*
     * All recorded events have been processed.
     */
    child_event_count = 0;
}