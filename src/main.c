#include "common.h"
#include "terminal.h"
#include "pty.h"
#include "session.h"
#include "hotkey.h"

/* SIGCHLD Handler */
static void sigchld_handler(int sig)
{
    (void)sig;
    int saved_errno = errno;
    pid_t pid;

    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        for (int i = 0; i < session_count; i++)
        {
            if (sessions[i].pid == pid)
            {
                sessions[i].is_active = 0;
                break;
            }
        }
    }
    errno = saved_errno;
}

static void setup_signals(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

int main(void)
{
    printf("=====================================\n");
    printf("     PtyMux - Terminal Multiplexer   \n");
    printf("  A POSIX PTY Session Manager in C   \n");
    printf("=====================================\n");
    printf(" Controls: Ctrl+B then:\n");
    printf("  'c' -> Create New Window\n");
    printf("  'n' -> Switch Next Window\n");
    printf("  'p' -> Switch Previous Window\n");
    printf("  'l' -> List All Active Windows\n");
    printf("  'q' -> Quit PtyMux\n");
    printf("=====================================\n\n");

    setup_signals();

    if (create_session(1) == -1)
    {
        fprintf(stderr, "Failed to start initial session.\n");
        return EXIT_FAILURE;
    }

    enable_raw_mode();

    char buffer[BUFFER_SIZE];
    int awaiting_command = 0;

    while (session_count > 0 && current_session_index >= 0)
    {
        fd_set readfds;
        FD_ZERO(&readfds);

        /* Monitor STDIN */
        FD_SET(STDIN_FILENO, &readfds);
        int maxfd = STDIN_FILENO;

        /* Monitor all active session master descriptors */
        for (int i = 0; i < session_count; i++)
        {
            if (sessions[i].is_active)
            {
                FD_SET(sessions[i].master_fd, &readfds);
                if (sessions[i].master_fd > maxfd)
                {
                    maxfd = sessions[i].master_fd;
                }
            }
        }

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            if (errno == EINTR) continue;
            perror("select failed");
            break;
        }

        /* 1. Handle User Keyboard Input */
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (bytes_read > 0)
            {
                for (ssize_t i = 0; i < bytes_read; i++)
                {
                    unsigned char c = buffer[i];

                    if (awaiting_command)
                    {
                        awaiting_command = 0;
                        if (handle_hotkey_command(c))
                        {
                            return 0;
                        }
                    }
                    else if (c == PREFIX_KEY)
                    {
                        awaiting_command = 1;
                    }
                    else if (current_session_index >= 0)
                    {
                        write(sessions[current_session_index].master_fd, &c, 1);
                    }
                }
            }
        }

        /* 2. Handle Shell Output for all sessions */
        for (int i = 0; i < session_count; i++)
        {
            if (sessions[i].is_active && FD_ISSET(sessions[i].master_fd, &readfds))
            {
                ssize_t bytes_read = read(sessions[i].master_fd, buffer, sizeof(buffer));
                if (bytes_read > 0)
                {
                    /* ONLY print output if session is currently active */
                    if (i == current_session_index)
                    {
                        write(STDOUT_FILENO, buffer, bytes_read);
                    }
                }
                else if (bytes_read == 0 || (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
                {
                    close_session(i);
                }
            }
        }
    }

    cleanup_all_sessions();
    disable_raw_mode();
    printf("\r\n[PtyMux] Session ended. Goodbye!\r\n");
    return 0;
}
