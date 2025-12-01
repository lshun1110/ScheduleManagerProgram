#include "common.h"
#include "ui_core.h"

void set_console()
{
	(void)_setmode(_fileno(stdout), _O_U16TEXT);
	(void)_setmode(_fileno(stdin), _O_U16TEXT);
	(void)_setmode(_fileno(stderr), _O_U16TEXT);

    Ui_InitConsole();
    set_cursor_visibility(0);

    SetConsoleTitleA("Schedule Management Program");

    SHORT width = 120;
    SHORT height = 30;

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD size;
    size.X = width;
    size.Y = height;
    SetConsoleScreenBufferSize(hOut, size);

    SMALL_RECT rect;
    rect.Left = 0;
    rect.Top = 0;
    rect.Right = width - 1;
    rect.Bottom = height - 1;


    SetConsoleWindowInfo(hOut, TRUE, &rect);
}