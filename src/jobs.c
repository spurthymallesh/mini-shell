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