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

void DrawColorBox(int x, int y, int w, int h, int color) {
    SetColor(color, COLOR_BLACK);
    for (int row = 0; row < h; row++) {
        goto_xy(x, y + row);
        for (int col = 0; col < w; col++) {
            // Use CP949-compatible square character for color box
            wprintf(L"\xa1\xe1");
        }
    }
    ResetColor();
}
