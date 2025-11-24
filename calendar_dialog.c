#include "common.h"
#include "calendar_dialog.h"
#include "calendar_mgr.h"
#include "ui_core.h"
#include "color_util.h"

int CalendarDialog_Add(const wchar_t* user_id) {
    wchar_t name[32] = L"";
    int color = COLOR_GREEN;
    int focused = 1;
    
    const wchar_t* color_names[] = {L"검정", L"파랑", L"초록", L"청록", L"빨강", L"자주", L"노랑", L"흰색"};
    int need_redraw = 1;
    
    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            draw_box(30, 8, 60, 14);
            goto_xy(32, 9);
            wprintf(L"━━━━━━━━━━ 새 캘린더 ━━━━━━━━━━");
            goto_xy(32, 11);
            wprintf(L"캘린더 이름 *");
            draw_box(32, 12, 40, 3);
            goto_xy(34, 13);
            if (name[0]) {
                wprintf(L"%-36ls", name);
            } else {
                SetColor(COLOR_WHITE - 8, COLOR_BLACK);
                wprintf(L"이름을 입력하세요");
                ResetColor();
            }
            goto_xy(32, 15);
            wprintf(L"색상");
            goto_xy(32, 16);
            wprintf(L"◀");
            DrawColorBox(35, 16, 3, 1, color);
            goto_xy(39, 16);
            wprintf(L"%ls", color_names[color]);
            goto_xy(55, 16);
            wprintf(L"▶");
            draw_box(38, 18, 12, 3);
            goto_xy(42, 19);
            wprintf(L"생성");
            draw_box(54, 18, 12, 3);
            goto_xy(58, 19);
            wprintf(L"취소");
            goto_xy(32, 21);
            wprintf(L"Tab: 다음 | Enter: 생성 | ESC: 취소");
            need_redraw = 0;
        }
        
        if (focused == 1 && name[0]) {
            goto_xy(34 + (int)wcslen(name), 13);
            set_cursor_visibility(1);
        } else {
            set_cursor_visibility(0);
        }
        
        UiInputEvent ev;
        Ui_WaitInput(&ev);
        
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x, my = ev.pos.y;
            if (my == 13 && mx >= 34 && mx < 70) { focused = 1; need_redraw = 1; }
            else if (mx >= 32 && mx <= 33 && my == 16) { color = (color - 1 + 8) % 8; need_redraw = 1; }
            else if (mx >= 55 && mx <= 56 && my == 16) { color = (color + 1) % 8; need_redraw = 1; }
            else if (my >= 18 && my <= 20 && mx >= 38 && mx <= 49) {
                if (name[0] == L'\0') {
                    goto_xy(32, 21);
                    wprintf(L"! 이름을 입력하세요!                    ");
                    Sleep(1500);
                    need_redraw = 1;
                    continue;
                }
                CalMgr_AddCalendar(user_id, name, color);
                set_cursor_visibility(0);
                return 1;
            }
            else if (my >= 18 && my <= 20 && mx >= 54 && mx <= 65) {
                set_cursor_visibility(0);
                return 0;
            }
        }
        else if (ev.type == UI_INPUT_KEY) {
            wchar_t ch = ev.key;
            if (ch == 27) { set_cursor_visibility(0); return 0; }
            if (ch == L'\t') { focused = 1 - focused; need_redraw = 1; }
            if (ch == L'\r' || ch == L'\n') {
                if (name[0] == L'\0') {
                    goto_xy(32, 21);
                    wprintf(L"! 이름을 입력하세요!                    ");
                    Sleep(1500);
                    need_redraw = 1;
                    continue;
                }
                CalMgr_AddCalendar(user_id, name, color);
                set_cursor_visibility(0);
                return 1;
            }
            if (focused == 1) {
                if (ch == L'\b') {
                    size_t len = wcslen(name);
                    if (len > 0) name[len - 1] = L'\0';
                    need_redraw = 1;
                } else if (iswprint(ch) || ch == L' ') {
                    size_t len = wcslen(name);
                    if (len < 31) {
                        name[len] = ch;
                        name[len + 1] = L'\0';
                        need_redraw = 1;
                    }
                }
            }
        }
    }
    return 0;
}

int CalendarDialog_Edit(Calendar* calendar) {
    return 0;
}

int CalendarDialog_ConfirmDelete(const Calendar* calendar) {
    Ui_ClearScreen();
    draw_box(30, 10, 60, 10);
    goto_xy(32, 12);
    wprintf(L"정말 삭제하시겠습니까?");
    goto_xy(32, 14);
    wprintf(L"캘린더: %ls", calendar->name);
    draw_box(40, 16, 15, 3);
    goto_xy(45, 17);
    wprintf(L"삭제");
    draw_box(60, 16, 15, 3);
    goto_xy(65, 17);
    wprintf(L"취소");
    UiRect rect_delete = {40, 16, 15, 3};
    UiRect rect_cancel = {60, 16, 15, 3};
    while (1) {
        UiInputEvent ev;
        Ui_WaitInput(&ev);
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            if (Ui_PointInRect(&rect_delete, ev.pos.x, ev.pos.y)) {
                return 1;
            } else if (Ui_PointInRect(&rect_cancel, ev.pos.x, ev.pos.y)) {
                return 0;
            }
        } else if (ev.type == UI_INPUT_KEY && ev.key == 27) {
            return 0;
        }
    }
}
