#include "common.h"
#include "ui_core.h"

static HANDLE hConsole;
static HANDLE hInput;

void Ui_InitConsole(void)
{
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    hInput = GetStdHandle(STD_INPUT_HANDLE);

    // 마우스 입력 활성화
    DWORD mode;
    GetConsoleMode(hInput, &mode);
    mode |= ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(hInput, mode);
}

void Ui_ClearScreen(void)
{
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
        return;

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);
    
    if (GetConsoleScreenBufferInfo(hConsole, &csbi))
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);

    SetConsoleCursorPosition(hConsole, coordScreen);
    
    // 마우스 모드 다시 활성화 (중요!)
    DWORD mode;
    GetConsoleMode(hInput, &mode);
    mode |= ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(hInput, mode);
}

int Ui_WaitInput(UiInputEvent* out)
{
    INPUT_RECORD rec;
    DWORD readCount;

    while (1) {
        if (!ReadConsoleInputW(hInput, &rec, 1, &readCount))
            continue;

        if (readCount == 0)
            continue;

        if (rec.EventType == MOUSE_EVENT) {
            MOUSE_EVENT_RECORD m = rec.Event.MouseEvent;

            if (m.dwEventFlags == 0 && (m.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
                out->type = UI_INPUT_MOUSE_LEFT;
                out->pos.x = m.dwMousePosition.X;
                out->pos.y = m.dwMousePosition.Y;
                out->key = 0;
                return 1;
            }
        }
        else if (rec.EventType == KEY_EVENT) {
            KEY_EVENT_RECORD k = rec.Event.KeyEvent;

            if (k.bKeyDown) {
                out->type = UI_INPUT_KEY;
                out->pos.x = out->pos.y = -1;
                out->key = k.uChar.UnicodeChar;
                return 1;
            }
        }
    }

    return 0;
}

void draw_box(int x, int y, int width, int height)
{
    set_cursor_visibility(0);
    if (width < 2 || height < 2) {
        return;
    }

    goto_xy(x, y);
    wprintf(L"┌");
    for (int i = 0; i < width - 2; i++)
        wprintf(L"─");
    wprintf(L"┐");

    for (int i = 1; i < height - 1; i++)
    {
        goto_xy(x, y + i);
        wprintf(L"│");

        goto_xy(x + width - 1, y + i);
        wprintf(L"│");
    }

    goto_xy(x, y + height - 1);
    wprintf(L"└");
    for (int i = 0; i < width - 2; i++)
        wprintf(L"─");
    wprintf(L"┘");
}

void set_cursor_visibility(int isVisible)
{
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = isVisible;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void goto_xy(int x, int y)
{
    COORD pos;
    pos.X = (SHORT)x;
    pos.Y = (SHORT)y;
    SetConsoleCursorPosition(hConsole, pos);
}

int Ui_PointInRect(const UiRect* r, int x, int y)
{
    return (x >= r->x && x < r->x + r->w &&
        y >= r->y && y < r->y + r->h);
}
