// ui_core.h
#ifndef UI_CORE_H
#define UI_CORE_H

#include "common.h"

typedef struct {
    int x;
    int y;
} UiPoint;

typedef enum {
    UI_INPUT_NONE = 0,
    UI_INPUT_MOUSE_LEFT,
    UI_INPUT_MOUSE_RIGHT,
    UI_INPUT_KEY
} UiInputType;

typedef struct {
    UiInputType type;
    UiPoint     pos;
    wchar_t     key;
} UiInputEvent;

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} UiRect;

int Ui_PointInRect(const UiRect* r, int x, int y);
void Ui_InitConsole();
void Ui_ClearScreen(void);  // system("cls") 대신 사용
void draw_box(int x, int y, int width, int height);
void set_cursor_visibility(int isVisible);
void goto_xy(int x, int y);
int Ui_WaitInput(UiInputEvent* out);

#endif
