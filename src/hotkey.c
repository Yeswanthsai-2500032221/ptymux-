#include "hotkey.h"

int handle_hotkey_command(unsigned char c)
{
    if (c == 'c' || c == 'C')
    {
        create_session(session_count + 1);
    }
    else if (c == 'n' || c == 'N')
    {
        if (session_count > 0)
        {
            switch_session((current_session_index + 1) % session_count);
        }
    }
    else if (c == 'p' || c == 'P')
    {
        if (session_count > 0)
        {
            switch_session((current_session_index - 1 + session_count) % session_count);
        }
    }
    else if (c == 'l' || c == 'L')
    {
        list_sessions();
    }
    else if (c == 'q' || c == 'Q')
    {
        printf("\r\n[PtyMux] Quitting cleanly...\r\n");
        cleanup_all_sessions();
        disable_raw_mode();
        return 1; /* Signal to exit main loop */
    }

    return 0;
}
