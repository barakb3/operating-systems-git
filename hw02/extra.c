int register_signal_handling();
void my_signal_handler(int signum, siginfo_t *info, void *ptr);

int prepare(void)
{

    if (signal(SIGINT, SIG_IGN) == SIG_ERR || register_signal_handling() == -1)
    {
        fprintf(stderr, "Failed registrating signal handler due to errno: %s", strerror(errno));
        return -1;
    }
    return 0;
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

void my_signal_handler(int signum, siginfo_t *info, void *ptr)
{
    pid_t pid = info->si_pid;
    wait_child_process(pid);
    return;
}