#ifndef SESSION_H
#define SESSION_H

#include "common.h"

typedef struct {
    int id;
    int master_fd;
    pid_t pid;
    char name[32];
    int is_active;
} session_t;

extern session_t sessions[MAX_SESSIONS];
extern int session_count;
extern int current_session_index;

int create_session(int id);
void close_session(int index);
void list_sessions(void);
void switch_session(int index);
void cleanup_all_sessions(void);

#endif /* SESSION_H */
