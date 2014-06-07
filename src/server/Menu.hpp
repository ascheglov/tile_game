#pragma once

#if defined _MSC_VER
#include <conio.h>
#else
#include "conio.hpp"
#endif

#include <iostream>

class Menu
{
public:
    void tick()
    {
        pollInput();

        switch (m_lastInput)
        {
        case 'q':
            m_quitRequested = true;
            break;

        case '?': case 'h':
            printHelp();
            break;
        }
    }

    bool quitRequested() const { return m_quitRequested; }

private:
    void pollInput()
    {
        m_lastInput = _kbhit() ? _getch() : '\0';
    }

    void printHelp()
    {
        std::cout << R"(
q - quit
h or ? - this message
)";
    }

    int m_lastInput;
    bool m_quitRequested{false};
};
