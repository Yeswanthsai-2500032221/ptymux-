#include "session.h"
#include "pty.h"

session_t sessions[MAX_SESSIONS];
int session_count = 0;
int current_session_index = -1;

int create_session(int id)
{
    if (session_count >= MAX_SESSIONS)
    {
        printf("\r\n[PtyMux Error] Maximum sessions reached (%d).\r\n", MAX_SESSIONS);
        return -1;
    }

    int master_fd = open_pty_master();
    if (master_fd == -1) return -1;

    pid_t pid = spawn_shell_process(master_fd);
    if (pid <= 0)
    {
        close(master_fd);
        return -1;
    }

    session_t *s = &sessions[session_count];
    s->id = id;
    s->master_fd = master_fd;
    s->pid = pid;
    snprintf(s->name, sizeof(s->name), "Session-%d (bash)", id);
    s->is_active = 1;

    int new_index = session_count;
    session_count++;
    current_session_index = new_index;

    printf("\r\n===================================================\r\n");
    printf(" [PtyMux] Active: %s (PID: %d)\r\n", s->name, pid);
    printf("===================================================\r\n");
    return new_index;
}

void close_session(int index)
{
    if (index < 0 || index >= session_count) return;

    session_t *s = &sessions[index];
    if (s->is_active)
    {
        close(s->master_fd);
        kill(s->pid, SIGKILL);
        waitpid(s->pid, NULL, WNOHANG);
        s->is_active = 0;
    }

    for (int i = index; i < session_count - 1; i++)
    {
        sessions[i] = sessions[i + 1];
    }
    session_count--;

    if (session_count > 0)
    {
        current_session_index = 0;
        printf("\r\n===================================================\r\n");
        printf(" [PtyMux] Switched to %s\r\n", sessions[current_session_index].name);
        printf("===================================================\r\n");
    }
    else
    {
        current_session_index = -1;
    }
}

void list_sessions(void)
{
    printf("\r\n--- Active PtyMux Sessions ---\r\n");
    for (int i = 0; i < session_count; i++)
    {
        printf(" [%d] %s (PID: %d) %s\r\n", 
               sessions[i].id, 
               sessions[i].name, 
               sessions[i].pid,
               (i == current_session_index) ? "<-- ACTIVE" : "");
    }
    printf("-------------------------------\r\n");
}

void switch_session(int index)
{
    if (index >= 0 && index < session_count)
    {
        current_session_index = index;
        printf("\r\n===================================================\r\n");
        printf(" [PtyMux] Switched to %s\r\n", sessions[current_session_index].name);
        printf("===================================================\r\n");
    }
}

void cleanup_all_sessions(void)
{
    for (int i = 0; i < session_count; i++)
    {
        if (sessions[i].is_active)
        {
            close(sessions[i].master_fd);
            kill(sessions[i].pid, SIGKILL);
            waitpid(sessions[i].pid, NULL, WNOHANG);
        }
    }
    session_count = 0;
    current_session_index = -1;
}
