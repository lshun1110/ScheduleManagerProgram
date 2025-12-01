#include "common.h"
#include "color_util.h"
#include "ui_core.h"

void SetColor(int fg, int bg) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (WORD)((bg << 4) | fg));
}

void ResetColor(void) {
    SetColor(COLOR_WHITE, COLOR_BLACK);
}
