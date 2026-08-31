#include "minishell.h"

void add_job(pid_t pid, job_state state, const char *command)
{
    if (job_count >= MAX_JOBS)
    {
        fprintf(stderr, "msh: maximum number of jobs reached\n");
        return;
    }

    jobs[job_count].job_id = job_count + 1;
    jobs[job_count].pid = pid;
    jobs[job_count].state = state;

    snprintf(jobs[job_count].command,
             sizeof(jobs[job_count].command),
             "%s",
             command);

    job_count++;
}


int find_job_by_pid(pid_t pid)
{
    int i;

    for (i = 0; i < job_count; i++)
    {
        if (jobs[i].pid == pid)
        {
            return i;
        }
    }

    return -1;
}


void remove_job(int index)
{
    int i;

    if (index < 0 || index >= job_count)
    {
        return;
    }

    for (i = index; i < job_count - 1; i++)
    {
        jobs[i] = jobs[i + 1];
    }

    job_count--;
}


void update_job_state(pid_t pid, job_state state)
{
    int index;

    index = find_job_by_pid(pid);

    if (index != -1)
    {
        jobs[index].state = state;
    }
}

int execute_jobs(void)
{
    int i;

    for (i = 0; i < job_count; i++)
    {
        if (jobs[i].state == JOB_RUNNING)
        {
            printf("[%d] Running    %s\n",
                   jobs[i].job_id,
                   jobs[i].command);
        }
        else if (jobs[i].state == JOB_STOPPED)
        {
            printf("[%d] Stopped    %s\n",
                   jobs[i].job_id,
                   jobs[i].command);
        }
    }

    return 0;
}

int execute_bg(char **args)
{
    int index = -1;
    pid_t pid;

    /*
     * bg without PID:
     * use the last stopped job.
     */
    if (args[1] == NULL)
    {
        int i;

        for (i = job_count - 1; i >= 0; i--)
        {
            if (jobs[i].state == JOB_STOPPED)
            {
                index = i;
                break;
            }
        }
    }
    else
    {
        /*
         * bg <pid>
         */
        pid = (pid_t)atoi(args[1]);

        index = find_job_by_pid(pid);
    }

    /*
     * No matching stopped job.
     */
    if (index == -1)
    {
        fprintf(stderr, "msh: bg: no stopped job\n");
        return 1;
    }

    /*
     * Only stopped jobs should be continued.
     */
    if (jobs[index].state != JOB_STOPPED)
    {
        fprintf(stderr,
                "msh: bg: process %d is not stopped\n",
                jobs[index].pid);

        return 1;
    }

    /*
     * Continue the stopped process.
     */
    if (kill(jobs[index].pid, SIGCONT) < 0)
    {
        perror("kill");
        return 1;
    }

    /*
     * Update job state.
     */
    jobs[index].state = JOB_RUNNING;

    printf("[%d] Running    %s\n",
           jobs[index].job_id,
           jobs[index].command);

    fflush(stdout);

    return 0;
}