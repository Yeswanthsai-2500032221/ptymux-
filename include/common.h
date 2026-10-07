#ifndef COMMON_H
#define COMMON_H

#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>

#define MAX_SESSIONS 5
#define BUFFER_SIZE 1024
#define PREFIX_KEY 2  /* Ctrl+B (ASCII 2) */

#endif /* COMMON_H */
