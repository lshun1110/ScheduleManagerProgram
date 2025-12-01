#include "common.h"
#include "calendar_dialog.h"
#include "calendar_mgr.h"
#include "file_io.h"
#include "ui_core.h"
#include "color_util.h"

static int LoadMyCalendars(const wchar_t* user_id, Calendar* buf, int max_count);
static int CalendarDialog_ConfirmSave(const wchar_t* name, int is_new);


// 현재 사용자(user_id)가 소유한, 삭제되지 않은 캘린더만 로드
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

// 반환값: 이 다이얼로그에서 "새 캘린더"를 하나라도 만들었으면 1, 아니면 0
int CalendarDialog_Add(const wchar_t* user_id)
{
    const wchar_t* color_names[] = {
        L"검정", L"파랑", L"초록", L"청록",
        L"빨강", L"자주", L"노랑", L"흰색"
    };

    Calendar calendars[100];
    int cal_count = LoadMyCalendars(user_id, calendars, 100);

    int selected = (cal_count > 0) ? 0 : -1;   // 현재 선택 인덱스
    int is_new = (cal_count == 0) ? 1 : 0;   // 1이면 "새로 만들기" 모드

    wchar_t edit_name[32];
    int edit_color;
    int edit_active;

    if (is_new) {
        edit_name[0] = L'\0';
        edit_color = COLOR_GREEN;
        edit_active = 1;
    }
    else {
        wcsncpy_s(edit_name, 32, calendars[selected].name, _TRUNCATE);
        edit_color = calendars[selected].color;
        edit_active = calendars[selected].is_active;
    }

    int focus_name = 1;   // 1이면 이름 입력 박스에 포커스
    int need_redraw = 1;
    int created_any = 0;   // 이 다이얼로그에서 새 캘린더를 만들었는지 여부

    while (1) {
        // ===== 화면 그리기 =====
        if (need_redraw) {
            Ui_ClearScreen();
            draw_box(0, 0, 119, 29);

            // 상단 헤더
            goto_xy(2, 1);
            wprintf(L"<- Back");
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

                // 색상 표시(깨지지 않게 단일 문자만 출력)
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
            goto_xy(detail_x + 5, detail_y + 8);
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
            int btn_h = 3;
            int btn_w = 12;
            int gap = 4;
            int btn_x_start = 29;

            int btn_new_x = btn_x_start;
            int btn_save_x = btn_x_start + (btn_w + gap);
            int btn_del_x = btn_save_x + (btn_w + gap);
            int btn_close_x = btn_del_x + (btn_w + gap);

            draw_box(btn_new_x, btn_y, btn_w, btn_h);
            goto_xy(btn_new_x + 4, btn_y + 1);
            wprintf(L"등록");

            draw_box(btn_save_x, btn_y, btn_w, btn_h);
            goto_xy(btn_save_x + 4, btn_y + 1);
            wprintf(L"저장");

            draw_box(btn_del_x, btn_y, btn_w, btn_h);
            goto_xy(btn_del_x + 4, btn_y + 1);
            wprintf(L"삭제");

            draw_box(btn_close_x, btn_y, btn_w, btn_h);
            goto_xy(btn_close_x + 4, btn_y + 1);
            wprintf(L"닫기");

            

            need_redraw = 0;

            // 커서 위치 (이름 입력칸에만 표시)
            if (focus_name) {
                set_cursor_visibility(1);
                goto_xy(detail_x + 2 + (int)wcslen(edit_name), detail_y + 4);
            }
            else {
                set_cursor_visibility(0);
            }
        }

        // ===== 클릭 판정용 Rect 정의 =====
        int list_x = 2;
        int list_y = 3;
        int row_start_y = list_y + 3;
        int max_rows = 12;

        int detail_x = 45;
        int detail_y = 3;

        int btn_y = 24;
        int btn_h = 3;
        int btn_w = 12;
        int gap = 4;
        int btn_x_start = 29;

        int btn_new_x = btn_x_start;
        int btn_save_x = btn_x_start + (btn_w + gap);
        int btn_del_x = btn_save_x + (btn_w + gap);
        int btn_close_x = btn_del_x + (btn_w + gap);

        UiRect rect_back = { 2, 1, 8, 1 }; // "<- Back"

        UiRect rect_name_box = { detail_x, detail_y + 3, 32, 3 };

        UiRect rect_color_left = { detail_x,      detail_y + 8, 1, 1 };
        UiRect rect_color_right = { detail_x + 20, detail_y + 8, 2, 1 };

        UiRect rect_active_toggle = { detail_x, detail_y + 11, 20, 1 };

        UiRect rect_btn_new = { btn_new_x,   btn_y, btn_w, btn_h };
        UiRect rect_btn_save = { btn_save_x,  btn_y, btn_w, btn_h };
        UiRect rect_btn_del = { btn_del_x,   btn_y, btn_w, btn_h };
        UiRect rect_btn_close = { btn_close_x, btn_y, btn_w, btn_h };

        // ===== 입력 처리 =====
        UiInputEvent ev;
        Ui_WaitInput(&ev);

        // --- 마우스 입력 ---
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x;
            int my = ev.pos.y;

            // 상단 Back
            if (Ui_PointInRect(&rect_back, mx, my)) {
                set_cursor_visibility(0);
                return created_any;
            }

            // 목록 각 행 클릭
            for (int i = 0; i < cal_count && i < max_rows; i++) {
                UiRect rect_row = {
                    list_x,
                    row_start_y + i,
                    35, // 대략 한 줄 폭
                    1
                };
                if (Ui_PointInRect(&rect_row, mx, my)) {
                    selected = i;
                    is_new = 0;
                    wcsncpy_s(edit_name, 32, calendars[selected].name, _TRUNCATE);
                    edit_color = calendars[selected].color;
                    edit_active = calendars[selected].is_active;
                    need_redraw = 1;
                    goto CONTINUE_LOOP;
                }
            }

            // 이름 박스 클릭 → 포커스
            if (Ui_PointInRect(&rect_name_box, mx, my)) {
                focus_name = 1;
                need_redraw = 1;
                goto CONTINUE_LOOP;
            }

            // 색상 ◀ 클릭
            if (Ui_PointInRect(&rect_color_left, mx, my)) {
                edit_color = (edit_color + 7) % 8;
                need_redraw = 1;
                goto CONTINUE_LOOP;
            }

            // 색상 ▶ 클릭
            if (Ui_PointInRect(&rect_color_right, mx, my)) {
                edit_color = (edit_color + 1) % 8;
                need_redraw = 1;
                goto CONTINUE_LOOP;
            }

            // 표시 여부 토글
            if (Ui_PointInRect(&rect_active_toggle, mx, my)) {
                edit_active = !edit_active;
                need_redraw = 1;
                goto CONTINUE_LOOP;
            }

            // [새로 만들기]
            if (Ui_PointInRect(&rect_btn_new, mx, my)) {
                is_new = 1;
                selected = -1;
                edit_name[0] = L'\0';
                edit_color = COLOR_GREEN;
                edit_active = 1;
                focus_name = 1;
                need_redraw = 1;
                goto CONTINUE_LOOP;
            }

            // [저장]  → 윈도우 팝업 확인 후 실제 저장
            if (Ui_PointInRect(&rect_btn_save, mx, my)) {
                if (edit_name[0] == L'\0') {
                    goto_xy(2, 26);
                    wprintf(L"! 이름을 입력하세요.                      ");
                    Sleep(1200);
                    need_redraw = 1;
                    goto CONTINUE_LOOP;
                }

                // 저장 확인 팝업 (기존 화면 위에 작은 창만 그림)
                if (!CalendarDialog_ConfirmSave(edit_name, is_new)) {
                    // 취소 눌렀으면 아무것도 안 하고 돌아감
                    need_redraw = 1;
                    goto CONTINUE_LOOP;
                }

                // 여기부터 실제 저장
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
                goto CONTINUE_LOOP;
            }

            // [삭제]  → 윈도우 팝업 확인 후 삭제
            if (Ui_PointInRect(&rect_btn_del, mx, my)) {
                if (selected < 0 || cal_count == 0) {
                    goto_xy(2, 26);
                    wprintf(L"! 삭제할 캘린더가 없습니다.               ");
                    Sleep(1200);
                    need_redraw = 1;
                    goto CONTINUE_LOOP;
                }

                if (CalendarDialog_ConfirmDelete(&calendars[selected])) {
                    CalMgr_DeleteCalendar(calendars[selected].calendar_id);
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
                }
                else {
                    need_redraw = 1;
                }
                goto CONTINUE_LOOP;
            }

            // [닫기]
            if (Ui_PointInRect(&rect_btn_close, mx, my)) {
                set_cursor_visibility(0);
                return created_any;
            }
        }
        // --- 키보드 입력 (이름 입력만) ---
        else if (ev.type == UI_INPUT_KEY) {
            wchar_t ch = ev.key;

            if (focus_name) {
                if (ch == L'\b') {  // Backspace
                    size_t len = wcslen(edit_name);
                    if (len > 0) {
                        edit_name[len - 1] = L'\0';
                        need_redraw = 1;
                    }
                }
                else if (iswprint(ch) || ch == L' ') {
                    size_t len = wcslen(edit_name);
                    if (len < 31) {
                        edit_name[len] = ch;
                        edit_name[len + 1] = L'\0';
                        need_redraw = 1;
                    }
                }
            }
        }

    CONTINUE_LOOP:
        continue;
    }

    // 형식상
    set_cursor_visibility(0);
    return created_any;
}

// 아직 별도 사용처 없음 (인터페이스 유지용)
int CalendarDialog_Edit(Calendar* calendar)
{
    (void)calendar;
    return 0;
}

// 삭제 확인 다이얼로그 (기존 화면 위에 작은 창만 그리기)
int CalendarDialog_ConfirmDelete(const Calendar* calendar)
{
    int console_w = 120;
    int console_h = 30;

    int dialog_w = 50;
    int dialog_h = 8;

    int dialog_x = (console_w - dialog_w) / 2;
    int dialog_y = (console_h - dialog_h) / 2;

    // 그림자 효과
    for (int i = 0; i < dialog_h; i++) {
        goto_xy(dialog_x + 2, dialog_y + 1 + i);
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        for (int j = 0; j < dialog_w; j++) {
            wprintf(L" ");
        }
        ResetColor();
    }

    // 다이얼로그 박스
    draw_box(dialog_x, dialog_y, dialog_w, dialog_h);

    goto_xy(dialog_x + 2, dialog_y + 1);
    wprintf(L"캘린더 삭제 확인");

    goto_xy(dialog_x + 2, dialog_y + 3);
    wprintf(L"정말 이 캘린더를 삭제하시겠습니까?");
    goto_xy(dialog_x + 2, dialog_y + 4);
    wprintf(L"캘린더: %ls", calendar->name);

    int btn_w = 10;
    int btn_h = 3;
    int gap = 4;

    int btn_area_w = btn_w * 2 + gap;
    int btn_x_start = dialog_x + (dialog_w - btn_area_w) / 2;
    int btn_y = dialog_y + dialog_h - 3;

    int btn_del_x = btn_x_start;
    int btn_can_x = btn_x_start + btn_w + gap;

    draw_box(btn_del_x, btn_y, btn_w, btn_h);
    goto_xy(btn_del_x + 2, btn_y + 1);
    wprintf(L"삭제");

    draw_box(btn_can_x, btn_y, btn_w, btn_h);
    goto_xy(btn_can_x + 2, btn_y + 1);
    wprintf(L"취소");

    UiRect rect_dialog = { dialog_x, dialog_y, dialog_w, dialog_h };
    UiRect rect_delete = { btn_del_x, btn_y, btn_w, btn_h };
    UiRect rect_cancel = { btn_can_x, btn_y, btn_w, btn_h };

    while (1) {
        UiInputEvent ev;
        Ui_WaitInput(&ev);

        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x;
            int my = ev.pos.y;

            if (!Ui_PointInRect(&rect_dialog, mx, my)) {
                continue;
            }

            if (Ui_PointInRect(&rect_delete, mx, my)) {
                return 1;
            }
            if (Ui_PointInRect(&rect_cancel, mx, my)) {
                return 0;
            }
        }
        else if (ev.type == UI_INPUT_KEY) {
            if (ev.key == L'\r' || ev.key == L'\n') {
                return 1;   // Enter → 삭제
            }
            if (ev.key == 27) {
                return 0;   // ESC → 취소
            }
        }
    }

    return 0;
}

// 저장 확인 다이얼로그 (새로 만들기 / 편집 공통)
static int CalendarDialog_ConfirmSave(const wchar_t* name, int is_new)
{
    int console_w = 120;
    int console_h = 30;

    int dialog_w = 50;
    int dialog_h = 8;

    int dialog_x = (console_w - dialog_w) / 2;
    int dialog_y = (console_h - dialog_h) / 2;

    // 그림자 효과
    for (int i = 0; i < dialog_h; i++) {
        goto_xy(dialog_x + 2, dialog_y + 1 + i);
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        for (int j = 0; j < dialog_w; j++) {
            wprintf(L" ");
        }
        ResetColor();
    }

    // 다이얼로그 박스
    draw_box(dialog_x, dialog_y, dialog_w, dialog_h);

    goto_xy(dialog_x + 2, dialog_y + 1);
    wprintf(L"캘린더 저장 확인");

    goto_xy(dialog_x + 2, dialog_y + 3);
    if (is_new) {
        wprintf(L"이 이름으로 새 캘린더를 저장하시겠습니까?");
    }
    else {
        wprintf(L"변경 내용을 저장하시겠습니까?");
    }
    goto_xy(dialog_x + 2, dialog_y + 4);
    wprintf(L"캘린더: %ls", name);

    int btn_w = 10;
    int btn_h = 3;
    int gap = 4;

    int btn_area_w = btn_w * 2 + gap;
    int btn_x_start = dialog_x + (dialog_w - btn_area_w) / 2;
    int btn_y = dialog_y + dialog_h - 3;

    int btn_ok_x = btn_x_start;
    int btn_can_x = btn_x_start + btn_w + gap;

    draw_box(btn_ok_x, btn_y, btn_w, btn_h);
    goto_xy(btn_ok_x + 2, btn_y + 1);
    wprintf(L"저장");

    draw_box(btn_can_x, btn_y, btn_w, btn_h);
    goto_xy(btn_can_x + 2, btn_y + 1);
    wprintf(L"취소");

    UiRect rect_dialog = { dialog_x, dialog_y, dialog_w, dialog_h };
    UiRect rect_ok = { btn_ok_x,  btn_y, btn_w, btn_h };
    UiRect rect_cancel = { btn_can_x, btn_y, btn_w, btn_h };

    while (1) {
        UiInputEvent ev;
        Ui_WaitInput(&ev);

        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x;
            int my = ev.pos.y;

            if (!Ui_PointInRect(&rect_dialog, mx, my)) {
                continue;
            }

            if (Ui_PointInRect(&rect_ok, mx, my)) {
                return 1;
            }
            if (Ui_PointInRect(&rect_cancel, mx, my)) {
                return 0;
            }
        }
        else if (ev.type == UI_INPUT_KEY) {
            if (ev.key == L'\r' || ev.key == L'\n') {
                return 1;   // Enter → 저장
            }
            if (ev.key == 27) {
                return 0;   // ESC → 취소
            }
        }
    }

    return 0;
}