#include "terminal.h"

static struct termios orig_termios;
static int raw_mode_enabled = 0;

void disable_raw_mode(void)
{
    if (raw_mode_enabled)
    {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_mode_enabled = 0;
    }
}

void enable_raw_mode(void)
{
    if (!isatty(STDIN_FILENO)) return;

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    {
        perror("tcgetattr failed");
        return;
    }

    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        perror("tcsetattr failed");
    }
    else
    {
        raw_mode_enabled = 1;
    }
}
