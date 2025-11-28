#include "common.h"
#include "calendar_dialog.h"
#include "calendar_mgr.h"
#include "file_io.h"
#include "ui_core.h"
#include "color_util.h"

// 현재 사용자(user_id)가 소유한, 살아있는 캘린더만 로드
static int LoadMyCalendars(const wchar_t* user_id, Calendar* buf, int max_count)
{
    Calendar all_cals[200];
    int total = FileIO_LoadCalendars(all_cals, 200);
    int count = 0;

    for (int i = 0; i < total && count < max_count; i++) {
        if (all_cals[i].is_deleted)
            continue;
        if (wcscmp(all_cals[i].user_id, user_id) != 0)
            continue;

        buf[count++] = all_cals[i];
    }
    return count;
}

// 한 화면에서 캘린더 C/R/U/D 관리
// 반환값: 이 다이얼로그에서 "새 캘린더"를 하나라도 만들었으면 1, 아니면 0
int CalendarDialog_Add(const wchar_t* user_id)
{
    const wchar_t* color_names[] = {
        L"검정", L"파랑", L"초록", L"청록",
        L"빨강", L"자주", L"노랑", L"흰색"
    };

    Calendar calendars[100];
    int cal_count = LoadMyCalendars(user_id, calendars, 100);

    int selected = (cal_count > 0) ? 0 : -1;   // 현재 선택된 인덱스
    int is_new = (cal_count == 0) ? 1 : 0;   // 1이면 "새로 만들기" 모드

    wchar_t edit_name[32] = L"";
    int edit_color = COLOR_GREEN;
    int edit_active = 1;

    if (selected >= 0) {
        wcsncpy_s(edit_name, 32, calendars[selected].name, _TRUNCATE);
        edit_color = calendars[selected].color;
        edit_active = calendars[selected].is_active;
        is_new = 0;
    }

    int focus_name = 1;   // 1이면 이름 입력 박스에 포커스
    int need_redraw = 1;
    int created_any = 0;   // 이 다이얼로그에서 새 캘린더를 만들었는지 여부

    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            draw_box(0, 0, 119, 29);

            // 상단 헤더
            goto_xy(2, 1);
            wprintf(L"<- 뒤로가기");
            goto_xy(50, 1);
            wprintf(L"캘린더 관리");

            // ===== 왼쪽: 내 캘린더 목록 =====
            int list_x = 2;
            int list_y = 3;

            goto_xy(list_x, list_y);
            wprintf(L"[내 캘린더 목록]");
            goto_xy(list_x, list_y + 1);
            wprintf(L"번호  이름                색상  표시");

            int row_start_y = list_y + 3;
            int max_rows = 12;   // 화면에 최대 12개까지 표시

            for (int i = 0; i < cal_count && i < max_rows; i++) {
                int y = row_start_y + i;
                goto_xy(list_x, y);

                const wchar_t* sel_mark = (i == selected) ? L">" : L" ";
                wprintf(L"%ls %2d. ", sel_mark, i + 1);

                // 색상 표시
                SetColor(calendars[i].color, COLOR_BLACK);
                wprintf(L"■");
                ResetColor();

                wprintf(L" %-14.14ls  %ls",
                    calendars[i].name,
                    calendars[i].is_active ? L"[v]" : L"[ ]");
            }

            // ===== 오른쪽: 선택 캘린더 상세 =====
            int detail_x = 45;
            int detail_y = 3;

            goto_xy(detail_x, detail_y);
            wprintf(L"[선택 캘린더 정보]");

            // 이름 입력
            goto_xy(detail_x, detail_y + 2);
            wprintf(L"이름");

            draw_box(detail_x, detail_y + 3, 32, 3);
            goto_xy(detail_x + 2, detail_y + 4);
            if (edit_name[0]) {
                wprintf(L"%-20ls", edit_name);
            }
            else {
                SetColor(COLOR_WHITE - 8, COLOR_BLACK);
                wprintf(L"이름을 입력하세요");
                ResetColor();
            }

            // 색상 선택
            goto_xy(detail_x, detail_y + 7);
            wprintf(L"색상");

            goto_xy(detail_x, detail_y + 8);
            wprintf(L"◀");
            goto_xy(detail_x + 6, detail_y + 8);
            SetColor(edit_color, COLOR_BLACK);
            wprintf(L"■");
            ResetColor();
            goto_xy(detail_x + 8, detail_y + 8);
            wprintf(L"%ls", color_names[edit_color]);
            goto_xy(detail_x + 20, detail_y + 8);
            wprintf(L"▶");

            // 표시 여부
            goto_xy(detail_x, detail_y + 11);
            wprintf(L"왼쪽 패널에 표시: %ls",
                edit_active ? L"[v]" : L"[ ]");

            // 소유자
            goto_xy(detail_x, detail_y + 13);
            wprintf(L"소유자: %ls", user_id);

            // ===== 아래 버튼들 =====
            int btn_y = 24;
            int btn_w = 12;
            int btn_h = 3;
            int gap = 4;
            int btn_x_start = 29;

            int btn_new_x = btn_x_start;
            int btn_save_x = btn_x_start + (btn_w + gap);
            int btn_del_x = btn_save_x + (btn_w + gap);
            int btn_close_x = btn_del_x + (btn_w + gap);

            draw_box(btn_new_x, btn_y, btn_w, btn_h);
            goto_xy(btn_new_x + 2, btn_y + 1);
            wprintf(L"새로 만들기");

            draw_box(btn_save_x, btn_y, btn_w, btn_h);
            goto_xy(btn_save_x + 3, btn_y + 1);
            wprintf(L"저장");

            draw_box(btn_del_x, btn_y, btn_w, btn_h);
            goto_xy(btn_del_x + 3, btn_y + 1);
            wprintf(L"삭제");

            draw_box(btn_close_x, btn_y, btn_w, btn_h);
            goto_xy(btn_close_x + 3, btn_y + 1);
            wprintf(L"닫기");

           
            need_redraw = 0;

            // 커서 위치
            if (focus_name) {
                set_cursor_visibility(1);
                goto_xy(detail_x + 2 + (int)wcslen(edit_name), detail_y + 4);
            }
            else {
                set_cursor_visibility(0);
            }
        }

        UiInputEvent ev;
        Ui_WaitInput(&ev);

        // 이벤트 처리용 좌표들 (그림과 동일하게 유지)
        int list_x = 2;
        int list_y = 3;
        int row_start_y = list_y + 3;
        int max_rows = 12;

        int detail_x = 45;
        int detail_y = 3;

        int btn_y = 24;
        int btn_w = 12;
        int btn_h = 3;
        int gap = 4;
        int btn_x_start = 29;

        int btn_new_x = btn_x_start;
        int btn_save_x = btn_x_start + (btn_w + gap);
        int btn_del_x = btn_save_x + (btn_w + gap);
        int btn_close_x = btn_del_x + (btn_w + gap);

        // ===================== 마우스 입력 =====================
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x;
            int my = ev.pos.y;

            // 상단 Back
            if (my == 1 && mx >= 2 && mx <= 8) {
                set_cursor_visibility(0);
                return created_any;
            }

            // 목록 클릭
            if (my >= row_start_y && my < row_start_y + max_rows &&
                mx >= list_x && mx <= list_x + 35) {

                int idx = my - row_start_y;
                if (idx >= 0 && idx < cal_count) {
                    selected = idx;
                    is_new = 0;
                    wcsncpy_s(edit_name, 32, calendars[selected].name, _TRUNCATE);
                    edit_color = calendars[selected].color;
                    edit_active = calendars[selected].is_active;
                    need_redraw = 1;
                }
                continue;
            }

            // 이름 박스 클릭
            if (my >= detail_y + 3 && my <= detail_y + 5 &&
                mx >= detail_x && mx <= detail_x + 31) {
                focus_name = 1;
                need_redraw = 1;
                continue;
            }

            // 색상 ◀ 클릭
            if (my == detail_y + 8 && mx == detail_x) {
                edit_color = (edit_color + 7) % 8;
                need_redraw = 1;
                continue;
            }
            // 색상 ▶ 클릭
            if (my == detail_y + 8 && mx >= detail_x + 20 && mx <= detail_x + 21) {
                edit_color = (edit_color + 1) % 8;
                need_redraw = 1;
                continue;
            }

            // 표시 토글
            if (my == detail_y + 11 && mx >= detail_x && mx <= detail_x + 20) {
                edit_active = !edit_active;
                need_redraw = 1;
                continue;
            }

            // [새로 만들기]
            if (my >= btn_y && my < btn_y + btn_h &&
                mx >= btn_new_x && mx < btn_new_x + btn_w) {

                is_new = 1;
                selected = -1;
                edit_name[0] = L'\0';
                edit_color = COLOR_GREEN;
                edit_active = 1;
                focus_name = 1;
                need_redraw = 1;
                continue;
            }

            // [저장]
            if (my >= btn_y && my < btn_y + btn_h &&
                mx >= btn_save_x && mx < btn_save_x + btn_w) {

                if (edit_name[0] == L'\0') {
                    goto_xy(2, 26);
                    wprintf(L"! 이름을 입력하세요.                         ");
                    Sleep(1200);
                    need_redraw = 1;
                    continue;
                }

                if (is_new) {
                    CalMgr_AddCalendar(user_id, edit_name, edit_color);
                    created_any = 1;
                }
                else if (selected >= 0) {
                    calendars[selected].color = edit_color;
                    calendars[selected].is_active = edit_active;
                    wcsncpy_s(calendars[selected].name, 32, edit_name, _TRUNCATE);
                    FileIO_UpdateCalendar(&calendars[selected]);
                }

                // 리스트 재로딩
                cal_count = LoadMyCalendars(user_id, calendars, 100);
                if (cal_count > 0) {
                    if (!is_new && selected >= 0 && selected < cal_count) {
                        // 기존 선택 유지
                    }
                    else {
                        selected = cal_count - 1;
                    }
                    wcsncpy_s(edit_name, 32, calendars[selected].name, _TRUNCATE);
                    edit_color = calendars[selected].color;
                    edit_active = calendars[selected].is_active;
                    is_new = 0;
                }
                else {
                    selected = -1;
                    is_new = 1;
                    edit_name[0] = L'\0';
                    edit_color = COLOR_GREEN;
                    edit_active = 1;
                }

                need_redraw = 1;
                continue;
            }

            // [삭제]
            if (my >= btn_y && my < btn_y + btn_h &&
                mx >= btn_del_x && mx < btn_del_x + btn_w) {

                if (selected < 0 || cal_count == 0) {
                    goto_xy(2, 26);
                    wprintf(L"! 삭제할 캘린더가 없습니다.                  ");
                    Sleep(1200);
                    need_redraw = 1;
                    continue;
                }

                int del_id = calendars[selected].calendar_id;
                goto_xy(2, 26);
                wprintf(L"정말 삭제하시겠습니까? (Y/N)");

                while (1) {
                    UiInputEvent ev2;
                    Ui_WaitInput(&ev2);
                    if (ev2.type == UI_INPUT_KEY) {
                        wchar_t ch2 = ev2.key;
                        if (ch2 == L'Y' || ch2 == L'y') {
                            CalMgr_DeleteCalendar(del_id);
                            created_any = 1;
                            cal_count = LoadMyCalendars(user_id, calendars, 100);
                            if (cal_count == 0) {
                                selected = -1;
                                is_new = 1;
                                edit_name[0] = L'\0';
                                edit_color = COLOR_GREEN;
                                edit_active = 1;
                            }
                            else {
                                if (selected >= cal_count)
                                    selected = cal_count - 1;
                                wcsncpy_s(edit_name, 32, calendars[selected].name, _TRUNCATE);
                                edit_color = calendars[selected].color;
                                edit_active = calendars[selected].is_active;
                                is_new = 0;
                            }
                            need_redraw = 1;
                            break;
                        }
                        else if (ch2 == L'N' || ch2 == L'n' || ch2 == 27) {
                            need_redraw = 1;
                            break;
                        }
                    }
                }
                continue;
            }

            // [닫기]
            if (my >= btn_y && my < btn_y + btn_h &&
                mx >= btn_close_x && mx < btn_close_x + btn_w) {
                set_cursor_visibility(0);
                return created_any;
            }
        }
        // ===================== 키보드 입력 =====================
        else if (ev.type == UI_INPUT_KEY) {
            wchar_t ch = ev.key;

            if (ch == 27) {   // ESC
                set_cursor_visibility(0);
                return created_any;
            }

            // W / S : 목록 위아래 이동
            if (ch == L'w' || ch == L'W') {
                if (cal_count > 0) {
                    if (selected <= 0)
                        selected = cal_count - 1;
                    else
                        selected--;
                    is_new = 0;
                    wcsncpy_s(edit_name, 32, calendars[selected].name, _TRUNCATE);
                    edit_color = calendars[selected].color;
                    edit_active = calendars[selected].is_active;
                    need_redraw = 1;
                }
                continue;
            }

            if (ch == L's' || ch == L'S') {
                if (cal_count > 0) {
                    if (selected >= cal_count - 1)
                        selected = 0;
                    else
                        selected++;
                    is_new = 0;
                    wcsncpy_s(edit_name, 32, calendars[selected].name, _TRUNCATE);
                    edit_color = calendars[selected].color;
                    edit_active = calendars[selected].is_active;
                    need_redraw = 1;
                }
                continue;
            }

            // Tab : 포커스 토글 (현재는 이름 입력만 사용)
            if (ch == L'\t') {
                focus_name = !focus_name;
                need_redraw = 1;
                continue;
            }

            // Enter = 저장
            if (ch == L'\r' || ch == L'\n') {
                if (edit_name[0] == L'\0') {
                    goto_xy(2, 26);
                    wprintf(L"! 이름을 입력하세요.                         ");
                    Sleep(1200);
                    need_redraw = 1;
                }
                else {
                    if (is_new) {
                        CalMgr_AddCalendar(user_id, edit_name, edit_color);
                        created_any = 1;
                    }
                    else if (selected >= 0) {
                        calendars[selected].color = edit_color;
                        calendars[selected].is_active = edit_active;
                        wcsncpy_s(calendars[selected].name, 32, edit_name, _TRUNCATE);
                        FileIO_UpdateCalendar(&calendars[selected]);
                    }

                    cal_count = LoadMyCalendars(user_id, calendars, 100);
                    if (cal_count > 0) {
                        if (!is_new && selected >= 0 && selected < cal_count) {
                            // 기존 선택 유지
                        }
                        else {
                            selected = cal_count - 1;
                        }
                        wcsncpy_s(edit_name, 32, calendars[selected].name, _TRUNCATE);
                        edit_color = calendars[selected].color;
                        edit_active = calendars[selected].is_active;
                        is_new = 0;
                    }
                    else {
                        selected = -1;
                        is_new = 1;
                        edit_name[0] = L'\0';
                        edit_color = COLOR_GREEN;
                        edit_active = 1;
                    }
                    need_redraw = 1;
                }
                continue;
            }

            // 이름 입력 처리
            if (focus_name) {
                if (ch == L'\b') {
                    size_t len = wcslen(edit_name);
                    if (len > 0) {
                        edit_name[len - 1] = L'\0';
                        need_redraw = 1;
                    }
                }
                else if (iswprint(ch)) {
                    size_t len = wcslen(edit_name);
                    if (len < 31) {
                        edit_name[len] = ch;
                        edit_name[len + 1] = L'\0';
                        need_redraw = 1;
                    }
                }
            }
        }
    }
}

// 아직은 별도 사용처가 없으므로 더미 구현
int CalendarDialog_Edit(Calendar* calendar)
{
    UNREFERENCED_PARAMETER(calendar);
    return 0;
}

int CalendarDialog_ConfirmDelete(const Calendar* calendar)
{
    UNREFERENCED_PARAMETER(calendar);
    return 0;
}