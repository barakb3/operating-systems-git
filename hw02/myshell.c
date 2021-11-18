#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

int prepare(void);
int process_arglist(int count, char **arglist);
int finalize(void);
int normal_exec(int count, char **arglist);
int background_exec(int count, char **arglist);
int redirection_exec(int count, char **arglist, char *filename);
int pipe_exec(int count_a, char **arglist_a, int count_b, char **arglist_b);
int wait_child_process(pid_t c_pid, int *exit_status);
int register_signal_handling();
int my_signal_handler(int signum, siginfo_t *info, void *ptr);

int prepare(void)
{
    if (signal(SIGINT, SIG_IGN) == SIG_ERR || register_signal_handling() == -1)
    {
        fprintf(stderr, "Failed registrating signal handler due to errno: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int process_arglist(int count, char **arglist)
{
    char *token;
    for (size_t i = 0; i < count; i++)
    {
        token = arglist[i];
        if (!strcmp(token, "&"))
        {
            if (i == 0)
                exit(1);
            return background_exec(i, arglist);
        }
        else if (!strcmp(token, ">"))
        {
            if (i == 0)
                exit(1);
            return redirection_exec(i, arglist, arglist[i + 1]);
        }
        else if (!strcmp(token, "|"))
        {
            arglist[i] = NULL;
            return pipe_exec(i, arglist, count - 1 - i, arglist + i + 1);
        }
    }
    return normal_ex(count, arglist);
}

int finalize(void)
{
    return 0;
}

int normal_exec(int count, char **arglist)
{
    int parent_status = 1; /* 0 means that the main process has encountered an error */
    int *exit_status;
    pid_t child_pid = fork();
    if (child_pid < 0)
    {
        /* Error when trying to create a new process */
        fprintf(stderr, "Failed forking due to errno: %s", strerror(errno));
        parent_status = 0;
    }
    else if (child_pid == 0)
    {
        /* Child's process */

        /* Restore default behavior of SIGINT */
        if (signal(SIGINT, SIG_DFL) == SIG_ERR)
        {
            fprintf(stderr, "Failed restoring SIGINT default behavior due to errno: %s", strerror(errno));
            exit(1);
        }

        /* Running the command*/
        if (execvp(arglist[0], arglist))
        {
            /* If reached here then executing arglist[0] failed */
            fprintf(stderr, "Failed in execvp due to errno: %s", strerror(errno));
            exit(1);
        }
    }
    else
    {
        /* Parent's process */
        if (wait_child_process(child_pid, exit_status) == 0)
        {
            /* An actual error that requires exiting the shell */
            parent_status = 0;
        }
    }
    return parent_status;
}

int background_exec(int count, char **arglist)
{
    int parent_status = 1; /* 0 means that the main process has encountered an error */
    pid_t c_pid;

    if (c_pid < 0)
    {
        /* Error when trying to create a new process */
        fprintf(stderr, "Failed forking due to errno: %s", strerror(errno));
        parent_status = 0;
    }
    else if (c_pid == 0)
    {
        /* Child's process */
        arglist[count] = NULL;
        execvp(arglist[0], arglist);
        /* If reached here then executing arglist[0] failed */
        exit(1);
    }
    else
    {
        /* Parent's process */
    }
    return parent_status;
}

int redirection_exec(int count, char **arglist, char *filename)
{
    int parent_status = 1; /* 0 means that the main process has encountered an error */
    int *exit_status;
    int fd;
    pid_t child_pid;

    fflush(stdout); /* Clear parent's stdout buffer before duplicating */
    child_pid = fork();
    if (child_pid < 0)
    {
        /* Error when trying to create a new process */
        fprintf(stderr, "Failed forking due to errno: %s", strerror(errno));
        parent_status = 0;
    }
    else if (child_pid == 0)
    {
        /* Child's process */
        if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO)) == -1)
        {
            fprintf(stderr, "Failed opening a file due to errno: %s", strerror(errno));
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);

        /* Restore default behavior of SIGINT */
        if (signal(SIGINT, SIG_DFL) == SIG_ERR)
        {
            fprintf(stderr, "Failed restoring SIGINT default behavior due to errno: %s", strerror(errno));
            exit(1);
        }

        arglist[count] = NULL;

        /* Running the command*/
        if (execvp(arglist[0], arglist))
        {
            /* If reached here then executing arglist[0] failed */
            fprintf(stderr, "Failed in execvp due to errno: %s", strerror(errno));
            exit(1);
        }
    }
    else
    {
        /* Parent's process */
        if (wait_child_process(child_pid, exit_status) == 0)
        {
            /* An actual error that requires exiting the shell */
            parent_status = 0;
        }
    }
    return parent_status;
}

int pipe_exec(int count_a, char **arglist_a, int count_b, char **arglist_b)
{
    int parent_status = 1; /* 0 means that the main process has encountered an error */
    int *exit_status;
    int pipefds[2], readerfds, writerfds;
    pid_t ca_pid = fork(), cb_pid;

    if (pipe(pipefds) < 0)
    {
        /* Error when trying to create a pipe */
        fprintf(stderr, "Failed piping due to errno: %s", strerror(errno));
        /* return negative number */
        parent_status = 0;
    }
    readerfds = pipefds[0];
    writerfds = pipefds[1];

    if (ca_pid < 0)
    {
        /* Error when trying to create a new process */
        fprintf(stderr, "Failed forking due to errno: %s", strerror(errno));
        parent_status = 0;
    }
    else if (ca_pid == 0)
    {
        /* First child's process (writes)*/

        /* Close unused read end */
        close(readerfds);
        dup2(writerfds, STDOUT_FILENO);
        close(writerfds);

        /* Restore default behavior of SIGINT */
        if (signal(SIGINT, SIG_DFL) == SIG_ERR)
        {
            fprintf(stderr, "Failed restoring SIGINT default behavior due to errno: %s", strerror(errno));
            exit(1);
        }

        /* Running the command*/
        if (execvp(arglist_a[0], arglist_a))
        {
            /* If reached here then executing arglist[0] failed */
            fprintf(stderr, "Failed in execvp due to errno: %s", strerror(errno));
            exit(1);
        }
    }
    else
    {
        /* Parent's process */
        /* Close unused write end for parent's and second child's processes */
        close(writerfds);
        cb_pid = fork();
        if (cb_pid < 0)
        {
            /* Error when trying to create a new process */
            fprintf(stderr, "Failed forking due to errno: %s", strerror(errno));
            parent_status = 0;
        }
        else if (cb_pid == 0)
        {
            /* Second child's process */

            /* Unused write end has already closed */
            dup2(readerfds, STDIN_FILENO);
            close(readerfds);

            /* Restore default behavior of SIGINT */
            if (signal(SIGINT, SIG_DFL) == SIG_ERR)
            {
                fprintf(stderr, "Failed restoring SIGINT default behavior due to errno: %s", strerror(errno));
                exit(1);
            }

            /* Running the command*/
            if (execvp(arglist_b[0], arglist_b))
            {
                /* If reached here then executing arglist[0] failed */
                fprintf(stderr, "Failed in execvp due to errno: %s", strerror(errno));
                exit(1);
            }
        }
        else
        {
            /* Again parent's process */
            /* Close read end */
            /* Unused write end has already closed */
            close(readerfds);
            if (wait_child_process(ca_pid, exit_status) == 0 || wait_child_process(cb_pid, exit_status) == 0)
            {
                /* An actual error that requires exiting the shell */
                parent_status = 0;
            }
        }
    }
    return parent_status;
}

int wait_child_process(pid_t c_pid, int *exit_status)
{
    if (waitpid(c_pid, exit_status, 0) == -1 && errno != ECHILD && errno != EINTR)
    {
        /* An actual error that requires exiting the shell */
        fprintf(stderr, "Failed executing wait() due to errno: %s", strerror(errno));
        return 0;
    }
    else
    {
        return 1;
    }
}

int register_signal_handling()
{
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));

    new_action.sa_sigaction = my_signal_handler;
    new_action.sa_flags = SA_SIGINFO;

    // Overwrite default behavior for ctrl+c
    return sigaction(SIGCHLD, &new_action, NULL);
}

int my_signal_handler(int signum, siginfo_t *info, void *ptr)
{
    pid_t pid = info->si_pid;
    int *exit_status;
    wait_child_process(pid, exit_status);
    return *exit_status;
}
