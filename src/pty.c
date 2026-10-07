#include "pty.h"

int open_pty_master(void)
{
    int master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd == -1)
    {
        perror("posix_openpt failed");
        return -1;
    }

    if (grantpt(master_fd) == -1 || unlockpt(master_fd) == -1)
    {
        perror("grantpt/unlockpt failed");
        close(master_fd);
        return -1;
    }

    return master_fd;
}

pid_t spawn_shell_process(int master_fd)
{
    char *slave_name = ptsname(master_fd);
    if (slave_name == NULL)
    {
        perror("ptsname failed");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        return -1;
    }

    if (pid == 0)
    {
        /* Child process setup */
        setsid();

        int slave_fd = open(slave_name, O_RDWR);
        if (slave_fd == -1)
        {
            perror("open slave_fd failed");
            exit(EXIT_FAILURE);
        }

        close(master_fd);

        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);

        if (slave_fd > STDERR_FILENO)
        {
            close(slave_fd);
        }

        setenv("TERM", "xterm-256color", 1);

        char *args[] = {"/bin/bash", NULL};
        execvp(args[0], args);

        perror("execvp failed");
        exit(EXIT_FAILURE);
    }

    /* Parent process */
    int flags = fcntl(master_fd, F_GETFL, 0);
    fcntl(master_fd, F_SETFL, flags | O_NONBLOCK);

    return pid;
}
