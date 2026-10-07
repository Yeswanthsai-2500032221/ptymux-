#ifndef PTY_H
#define PTY_H

#include "common.h"

int open_pty_master(void);
pid_t spawn_shell_process(int master_fd);

#endif /* PTY_H */
