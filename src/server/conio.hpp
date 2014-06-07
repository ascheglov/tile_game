#pragma once

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

inline void disable_buffering()
{
    static bool _ = ([]{
        termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
        setbuf(stdin, NULL);
        return false;
    })();
    (void)_;
}

inline int _kbhit()
{
    disable_buffering();

    int bytesWaiting;
    ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

inline int _getch()
{
    disable_buffering();

    char ch;
    return read(STDIN_FILENO, &ch, 1) == 1 ? ch : 0;    
}
