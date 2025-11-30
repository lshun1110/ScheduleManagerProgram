#include "common.h"
#include "schedule_dialog.h"
#include "ui_core.h"
#include "color_util.h"
#include "file_io.h"

typedef enum {
    FIELD_CALENDAR = 0,
    FIELD_TITLE,
    FIELD_LOCATION,
    FIELD_START_DATE,
    FIELD_START_TIME,
    FIELD_END_DATE,
    FIELD_END_TIME,
    FIELD_MEMO,
    FIELD_ALLDAY,
    FIELD_REPEAT,
    FIELD_SAVE,
    FIELD_CANCEL,
    FIELD_MAX
} DialogField;

static void DrawDialog(const Calendar* calendars, int cal_count, int selected_cal,
    const wchar_t* title, const wchar_t* location,
    const wchar_t* start_date, const wchar_t* start_time,
    const wchar_t* end_date, const wchar_t* end_time,
    const wchar_t* memo, int is_allday, int repeat_type,
    DialogField focused) {
    draw_box(0, 0, 119, 29);

    goto_xy(12, 3);
    wprintf(L"━━━━━━━━━━━━━━━━━━ 일정 등록 ━━━━━━━━━━━━━━━━━━");

    // 캘린더 선택
    goto_xy(12, 5);
    wprintf(L"캘린더 *");
    draw_box(23, 5, 30, 3);
    goto_xy(25, 6);
    if (selected_cal >= 0 && selected_cal < cal_count) {
        SetColor(calendars[selected_cal].color, COLOR_BLACK);
        wprintf(L"■");
        ResetColor();
        wprintf(L" %ls", calendars[selected_cal].name);
    }
    goto_xy(55, 6);
    wprintf(L"◀ ▶");

    // 제목
    goto_xy(12, 8);
    wprintf(L"제목 *");
    draw_box(23, 8, 70, 3);
    goto_xy(25, 9);
    if (title[0]) {
        wprintf(L"%-50ls", title);
    }
    else if (focused != FIELD_TITLE) {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"일정 제목을 입력하세요");
        ResetColor();
    }

    // 장소
    goto_xy(12, 11);
    wprintf(L"장소");
    draw_box(23, 11, 70, 3);
    goto_xy(25, 12);
    if (location[0]) {
        wprintf(L"%-50ls", location);
    }
    else if (focused != FIELD_LOCATION) {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"장소 (일정 위치)");
        ResetColor();
    }

    // 시작 날짜/시간
    goto_xy(12, 14);
    wprintf(L"시작");
    draw_box(23, 14, 15, 3);
    goto_xy(25, 15);
    if (start_date[0]) {
        wprintf(L"%-11ls", start_date);
    }
    else {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"YYYY-MM-DD");
        ResetColor();
    }

    draw_box(40, 14, 10, 3);
    goto_xy(42, 15);
    if (is_allday) {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"종일");
        ResetColor();
    }
    else if (start_time[0]) {
        wprintf(L"%-6ls", start_time);
    }
    else {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"HH:MM");
        ResetColor();
    }

    // 종료 날짜/시간
    goto_xy(52, 14);
    wprintf(L"종료");
    draw_box(58, 14, 15, 3);
    goto_xy(60, 15);
    if (end_date[0]) {
        wprintf(L"%-11ls", end_date);
    }
    else {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"YYYY-MM-DD");
        ResetColor();
    }

    draw_box(75, 14, 10, 3);
    goto_xy(77, 15);
    if (is_allday) {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"종일");
        ResetColor();
    }
    else if (end_time[0]) {
        wprintf(L"%-6ls", end_time);
    }
    else {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"HH:MM");
        ResetColor();
    }

    // 메모
    goto_xy(12, 17);
    wprintf(L"메모");
    draw_box(23, 17, 70, 4);
    goto_xy(25, 18);
    if (memo[0]) {
        wprintf(L"%-50ls", memo);
    }
    else if (focused != FIELD_MEMO) {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"일정 메모");
        ResetColor();
    }

    // 종일 여부
    goto_xy(12, 21);
    wprintf(is_allday ? L"[v] 종일 일정" : L"[ ] 종일 일정");

    // 반복
    goto_xy(40, 21);
    const wchar_t* repeat_names[] = { L"없음", L"매일", L"매주", L"매월" };
    wprintf(L"반복: ◀ %ls ▶", repeat_names[repeat_type]);

    // 버튼
    draw_box(35, 23, 15, 3);
    goto_xy(40, 24);
    wprintf(L"저 장");

    draw_box(55, 23, 15, 3);
    goto_xy(60, 24);
    wprintf(L"삭 제");

}

int ScheduleDialog_Add(const Calendar* calendars, int cal_count,
    const struct tm* default_date,
    Schedule* out_schedule) {
    int selected_cal = 0;
    wchar_t title[64] = L"";
    wchar_t location[64] = L"";
    wchar_t start_date[16] = L"";
    wchar_t start_time[8] = L"09:00";
    wchar_t end_date[16] = L"";
    wchar_t end_time[8] = L"10:00";
    wchar_t memo[256] = L"";
    int is_allday = 0;
    int repeat_type = 0;

    DialogField focused = FIELD_TITLE;

    // 기본값 설정
    if (default_date) {
        swprintf_s(start_date, 16, L"%04d-%02d-%02d",
            default_date->tm_year + 1900,
            default_date->tm_mon + 1,
            default_date->tm_mday);
        wcsncpy_s(end_date, 16, start_date, _TRUNCATE);
    }

    // 클릭 영역
    UiRect rect_cal_left = { 55, 6, 2, 1 };
    UiRect rect_cal_right = { 58, 6, 2, 1 };
    UiRect rect_title = { 25, 9, 66, 1 };
    UiRect rect_location = { 25, 12, 66, 1 };
    UiRect rect_start_date = { 25, 15, 11, 1 };
    UiRect rect_start_time = { 42, 15, 6, 1 };
    UiRect rect_end_date = { 60, 15, 11, 1 };
    UiRect rect_end_time = { 77, 15, 6, 1 };
    UiRect rect_memo = { 25, 18, 66, 1 };
    UiRect rect_allday = { 12, 21, 13, 1 };
    UiRect rect_repeat_left = { 47, 21, 2, 1 };
    UiRect rect_repeat_right = { 57, 21, 2, 1 };
    UiRect rect_save = { 35, 23, 15, 3 };
    // draw back button
    goto_xy(1, 1);
    wprintf(L"<- Back");
    UiRect rect_cancel = { 55, 23, 15, 3 };

    int need_redraw = 1;

    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            DrawDialog(calendars, cal_count, selected_cal,
                title, location, start_date, start_time,
                end_date, end_time, memo, is_allday, repeat_type, focused);
            goto_xy(1, 1);
            wprintf(L"<- Back");
            need_redraw = 0;
        }

        // 커서 위치
        if (focused == FIELD_TITLE && title[0]) {
            goto_xy(25 + (int)wcslen(title), 9);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_LOCATION && location[0]) {
            goto_xy(25 + (int)wcslen(location), 12);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_START_DATE) {
            goto_xy(25 + (int)wcslen(start_date), 15);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_START_TIME && !is_allday) {
            goto_xy(42 + (int)wcslen(start_time), 15);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_END_DATE) {
            goto_xy(60 + (int)wcslen(end_date), 15);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_END_TIME && !is_allday) {
            goto_xy(77 + (int)wcslen(end_time), 15);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_MEMO && memo[0]) {
            goto_xy(25 + (int)wcslen(memo), 18);
            set_cursor_visibility(1);
        }
        else {
            set_cursor_visibility(0);
        }

        UiInputEvent ev;
        Ui_WaitInput(&ev);

        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            // back button click
            if (ev.pos.y == 1 && ev.pos.x >= 1 && ev.pos.x <= 7) {
                return 0;
            }
            int mx = ev.pos.x, my = ev.pos.y;
            // back button click
            if (my == 1 && mx >= 1 && mx <= 7) {
                set_cursor_visibility(0);
                return 0;
            }

            // 캘린더 선택
            if (Ui_PointInRect(&rect_cal_left, mx, my)) {
                selected_cal = (selected_cal - 1 + cal_count) % cal_count;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_cal_right, mx, my)) {
                selected_cal = (selected_cal + 1) % cal_count;
                need_redraw = 1;
            }
            // 필드 클릭
            else if (Ui_PointInRect(&rect_title, mx, my)) {
                focused = FIELD_TITLE;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_location, mx, my)) {
                focused = FIELD_LOCATION;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_start_date, mx, my)) {
                focused = FIELD_START_DATE;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_start_time, mx, my)) {
                focused = FIELD_START_TIME;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_end_date, mx, my)) {
                focused = FIELD_END_DATE;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_end_time, mx, my)) {
                focused = FIELD_END_TIME;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_memo, mx, my)) {
                focused = FIELD_MEMO;
                need_redraw = 1;
            }
            // 종일
            else if (Ui_PointInRect(&rect_allday, mx, my)) {
                is_allday = !is_allday;
                need_redraw = 1;
            }
            // 반복
            else if (Ui_PointInRect(&rect_repeat_left, mx, my)) {
                repeat_type = (repeat_type - 1 + 4) % 4;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_repeat_right, mx, my)) {
                repeat_type = (repeat_type + 1) % 4;
                need_redraw = 1;
            }
            // 저장
            else if (Ui_PointInRect(&rect_save, mx, my)) {
                if (title[0] == L'\0') {
                    goto_xy(12, 27);
                    wprintf(L"! 제목을 입력하세요!                              ");
                    Sleep(1500);
                    need_redraw = 1;
                    continue;
                }

                memset(out_schedule, 0, sizeof(Schedule));
                out_schedule->calendar_id = calendars[selected_cal].calendar_id;
                wcsncpy_s(out_schedule->title, 64, title, _TRUNCATE);
                wcsncpy_s(out_schedule->location, 64, location, _TRUNCATE);
                wcsncpy_s(out_schedule->memo, 256, memo, _TRUNCATE);
                out_schedule->is_all_day = is_allday;
                out_schedule->repeat_type = (RepeatType)repeat_type;

                int y, m, d, h, min;
                if (swscanf_s(start_date, L"%d-%d-%d", &y, &m, &d) == 3) {
                    out_schedule->start_time.tm_year = y - 1900;
                    out_schedule->start_time.tm_mon = m - 1;
                    out_schedule->start_time.tm_mday = d;
                }
                if (!is_allday && swscanf_s(start_time, L"%d:%d", &h, &min) == 2) {
                    out_schedule->start_time.tm_hour = h;
                    out_schedule->start_time.tm_min = min;
                }

                if (swscanf_s(end_date, L"%d-%d-%d", &y, &m, &d) == 3) {
                    out_schedule->end_time.tm_year = y - 1900;
                    out_schedule->end_time.tm_mon = m - 1;
                    out_schedule->end_time.tm_mday = d;
                }
                if (!is_allday && swscanf_s(end_time, L"%d:%d", &h, &min) == 2) {
                    out_schedule->end_time.tm_hour = h;
                    out_schedule->end_time.tm_min = min;
                }

                set_cursor_visibility(0);
                return 1;
            }
            // 취소
            else if (Ui_PointInRect(&rect_cancel, mx, my)) {
                set_cursor_visibility(0);
                return 0;
            }
        }
        else if (ev.type == UI_INPUT_KEY) {
            wchar_t ch = ev.key;

            if (ch == 27) { // ESC
                set_cursor_visibility(0);
                return 0;
            }

            if (ch == L'\t') {
                focused = (DialogField)(((int)focused + 1) % FIELD_MAX);
                if (focused >= FIELD_SAVE) focused = FIELD_CALENDAR;
                need_redraw = 1;
                continue;
            }

            if (ch == L'\r' || ch == L'\n') { // Enter
                if (title[0] == L'\0') {
                    goto_xy(12, 27);
                    wprintf(L"! 제목을 입력하세요!                              ");
                    Sleep(1500);
                    need_redraw = 1;
                    continue;
                }

                memset(out_schedule, 0, sizeof(Schedule));
                out_schedule->calendar_id = calendars[selected_cal].calendar_id;
                wcsncpy_s(out_schedule->title, 64, title, _TRUNCATE);
                wcsncpy_s(out_schedule->location, 64, location, _TRUNCATE);
                wcsncpy_s(out_schedule->memo, 256, memo, _TRUNCATE);
                out_schedule->is_all_day = is_allday;
                out_schedule->repeat_type = (RepeatType)repeat_type;

                int y, m, d, h, min;
                if (swscanf_s(start_date, L"%d-%d-%d", &y, &m, &d) == 3) {
                    out_schedule->start_time.tm_year = y - 1900;
                    out_schedule->start_time.tm_mon = m - 1;
                    out_schedule->start_time.tm_mday = d;
                }
                if (!is_allday && swscanf_s(start_time, L"%d:%d", &h, &min) == 2) {
                    out_schedule->start_time.tm_hour = h;
                    out_schedule->start_time.tm_min = min;
                }

                if (swscanf_s(end_date, L"%d-%d-%d", &y, &m, &d) == 3) {
                    out_schedule->end_time.tm_year = y - 1900;
                    out_schedule->end_time.tm_mon = m - 1;
                    out_schedule->end_time.tm_mday = d;
                }
                if (!is_allday && swscanf_s(end_time, L"%d:%d", &h, &min) == 2) {
                    out_schedule->end_time.tm_hour = h;
                    out_schedule->end_time.tm_min = min;
                }

                set_cursor_visibility(0);
                return 1;
            }

            if (ch == L'\b') {
                wchar_t* buf = NULL;
                if (focused == FIELD_TITLE) buf = title;
                else if (focused == FIELD_LOCATION) buf = location;
                else if (focused == FIELD_START_DATE) buf = start_date;
                else if (focused == FIELD_START_TIME) buf = start_time;
                else if (focused == FIELD_END_DATE) buf = end_date;
                else if (focused == FIELD_END_TIME) buf = end_time;
                else if (focused == FIELD_MEMO) buf = memo;

                if (buf) {
                    size_t len = wcslen(buf);
                    if (len > 0) buf[len - 1] = L'\0';
                    need_redraw = 1;
                }
            }
            else if (iswprint(ch) || ch == L' ') {
                wchar_t* buf = NULL;
                int max_len = 0;

                if (focused == FIELD_TITLE) { buf = title; max_len = 63; }
                else if (focused == FIELD_LOCATION) { buf = location; max_len = 63; }
                else if (focused == FIELD_START_DATE) { buf = start_date; max_len = 15; }
                else if (focused == FIELD_START_TIME && !is_allday) { buf = start_time; max_len = 7; }
                else if (focused == FIELD_END_DATE) { buf = end_date; max_len = 15; }
                else if (focused == FIELD_END_TIME && !is_allday) { buf = end_time; max_len = 7; }
                else if (focused == FIELD_MEMO) { buf = memo; max_len = 255; }

                if (buf) {
                    size_t len = wcslen(buf);
                    if (len < max_len) {
                        buf[len] = ch;
                        buf[len + 1] = L'\0';
                        need_redraw = 1;
                    }
                }
            }
        }
    }

    return 0;
}

int ScheduleDialog_Edit_Stub(const Calendar* calendars, int cal_count, Schedule* schedule) {
    // TODO: 수정은 Add와 유사하지만 기존 값 로드
    return 0;
}

int ScheduleDialog_ConfirmDelete(const Schedule* schedule)
{
    
    //  큰 박스 안에 가운데 정렬된 작은 확인 다이얼로그 박스
    int dialog_w = 60;
    int dialog_h = 9;

    int dialog_x = 2 + (116 - dialog_w) / 2;
    int dialog_y = 1 + (28 - dialog_h) / 2;

    // 안쪽 다이얼로그 박스 테두리
    draw_box(dialog_x, dialog_y, dialog_w, dialog_h);

    for (int y = dialog_y + 1; y < dialog_y + dialog_h - 1; y++) {
        goto_xy(dialog_x + 1, y);
        for (int x = 0; x < dialog_w - 2; x++) {
            wprintf(L" ");
        }
    }

    // 다시 테두리 한 번 더 그려서 (안 채운 공백 위에) 라인 깨끗하게
    draw_box(dialog_x, dialog_y, dialog_w, dialog_h);

    // 제목/메시지 (캘린더 쪽 스타일 그대로)
    goto_xy(dialog_x + 2, dialog_y + 1);
    wprintf(L"일정 삭제 확인");

    goto_xy(dialog_x + 2, dialog_y + 3);
    wprintf(L"정말 이 일정을 삭제하시겠습니까?");

    goto_xy(dialog_x + 2, dialog_y + 4);
    wprintf(L"일정: %ls", schedule->title);

    // 4. 버튼 배치: [삭제] [취소]
    int btn_w = 10;
    int btn_h = 3;
    int gap = 4;
    int btn_y = dialog_y + dialog_h - 3;

    int btn_total_w = btn_w * 2 + gap;
    int btn_x_start = dialog_x + (dialog_w - btn_total_w) / 2;

    int btn_del_x = btn_x_start;
    int btn_can_x = btn_x_start + btn_w + gap;

    // [삭제] 버튼
    draw_box(btn_del_x, btn_y, btn_w, btn_h);
    goto_xy(btn_del_x + 2, btn_y + 1);
    wprintf(L"삭제");

    // [취소] 버튼
    draw_box(btn_can_x, btn_y, btn_w, btn_h);
    goto_xy(btn_can_x + 2, btn_y + 1);
    wprintf(L"취소");

    // 마우스 클릭 판정용 영역
    UiRect rect_delete = { btn_del_x, btn_y, btn_w, btn_h };
    UiRect rect_cancel = { btn_can_x, btn_y, btn_w, btn_h };

    UiInputEvent ev;

    while (1)
    {
        if (!Ui_WaitInput(&ev))
            continue;

        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x;
            int my = ev.pos.y;

            if (Ui_PointInRect(&rect_delete, mx, my)) {
                // [삭제] 클릭
                return 1;
            }
            if (Ui_PointInRect(&rect_cancel, mx, my)) {
                // [취소] 클릭
                return 0;
            }
        }

    }

    return 0;
}

int ScheduleDialog_Edit(const Calendar* calendars, int cal_count, Schedule* schedule) {
    int selected_cal = 0;
    for (int i = 0; i < cal_count; i++) {
        if (calendars[i].calendar_id == schedule->calendar_id) {
            selected_cal = i;
            break;
        }
    }
    wchar_t title[64] = L"";
    wcsncpy_s(title, 64, schedule->title, _TRUNCATE);
    wchar_t location[64] = L"";
    wcsncpy_s(location, 64, schedule->location, _TRUNCATE);
    wchar_t memo[256] = L"";
    wcsncpy_s(memo, 256, schedule->memo, _TRUNCATE);
    wchar_t start_date[16] = L"";
    wchar_t start_time[8] = L"";
    wchar_t end_date[16] = L"";
    wchar_t end_time[8] = L"";
    int is_allday = schedule->is_all_day;
    int repeat_type = schedule->repeat_type;
    // format dates
    swprintf_s(start_date, 16, L"%04d-%02d-%02d",
        schedule->start_time.tm_year + 1900,
        schedule->start_time.tm_mon + 1,
        schedule->start_time.tm_mday);
    swprintf_s(end_date, 16, L"%04d-%02d-%02d",
        schedule->end_time.tm_year + 1900,
        schedule->end_time.tm_mon + 1,
        schedule->end_time.tm_mday);
    if (!is_allday) {
        swprintf_s(start_time, 8, L"%02d:%02d",
            schedule->start_time.tm_hour,
            schedule->start_time.tm_min);
        swprintf_s(end_time, 8, L"%02d:%02d",
            schedule->end_time.tm_hour,
            schedule->end_time.tm_min);
    }
    else {
        start_time[0] = L'\0';
        end_time[0] = L'\0';
    }
    DialogField focused = FIELD_TITLE;

    UiRect rect_cal_left = { 55, 6, 2, 1 };
    UiRect rect_cal_right = { 58, 6, 2, 1 };
    UiRect rect_title = { 25, 9, 66, 1 };
    UiRect rect_location = { 25, 12, 66, 1 };
    UiRect rect_start_date = { 25, 15, 11, 1 };
    UiRect rect_start_time = { 42, 15, 6, 1 };
    UiRect rect_end_date = { 60, 15, 11, 1 };
    UiRect rect_end_time = { 77, 15, 6, 1 };
    UiRect rect_memo = { 25, 18, 66, 1 };
    UiRect rect_allday = { 12, 21, 13, 1 };
    UiRect rect_repeat_left = { 47, 21, 2, 1 };
    UiRect rect_repeat_right = { 57, 21, 2, 1 };
    UiRect rect_save = { 35, 23, 15, 3 };
    UiRect rect_cancel = { 55, 23, 15, 3 };

    int need_redraw = 1;
    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            DrawDialog(calendars, cal_count, selected_cal,
                title, location, start_date, start_time,
                end_date, end_time, memo, is_allday, repeat_type, focused);
            goto_xy(1, 1);
            wprintf(L"<- Back");
            need_redraw = 0;
        }
        // cursor position
        if (focused == FIELD_TITLE && title[0]) {
            goto_xy(25 + (int)wcslen(title), 9);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_LOCATION && location[0]) {
            goto_xy(25 + (int)wcslen(location), 12);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_START_DATE) {
            goto_xy(25 + (int)wcslen(start_date), 15);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_START_TIME && !is_allday) {
            goto_xy(42 + (int)wcslen(start_time), 15);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_END_DATE) {
            goto_xy(60 + (int)wcslen(end_date), 15);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_END_TIME && !is_allday) {
            goto_xy(77 + (int)wcslen(end_time), 15);
            set_cursor_visibility(1);
        }
        else if (focused == FIELD_MEMO && memo[0]) {
            goto_xy(25 + (int)wcslen(memo), 18);
            set_cursor_visibility(1);
        }
        else {
            set_cursor_visibility(0);
        }
        UiInputEvent ev;
        Ui_WaitInput(&ev);
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            // back button
            if (ev.pos.y == 1 && ev.pos.x >= 1 && ev.pos.x <= 7) {
                return 0;
            }
            int mx = ev.pos.x, my = ev.pos.y;
            if (Ui_PointInRect(&rect_cal_left, mx, my)) {
                selected_cal = (selected_cal - 1 + cal_count) % cal_count;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_cal_right, mx, my)) {
                selected_cal = (selected_cal + 1) % cal_count;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_title, mx, my)) {
                focused = FIELD_TITLE;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_location, mx, my)) {
                focused = FIELD_LOCATION;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_start_date, mx, my)) {
                focused = FIELD_START_DATE;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_start_time, mx, my)) {
                focused = FIELD_START_TIME;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_end_date, mx, my)) {
                focused = FIELD_END_DATE;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_end_time, mx, my)) {
                focused = FIELD_END_TIME;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_memo, mx, my)) {
                focused = FIELD_MEMO;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_allday, mx, my)) {
                is_allday = !is_allday;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_repeat_left, mx, my)) {
                repeat_type = (repeat_type - 1 + 4) % 4;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_repeat_right, mx, my)) {
                repeat_type = (repeat_type + 1) % 4;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_save, mx, my)) {
                if (title[0] == L'\0') {
                    goto_xy(12, 27);
                    wprintf(L"! 제목을 입력해 주세요!                              ");
                    Sleep(1500);
                    need_redraw = 1;
                    continue;
                }

                // 현재 입력값을 schedule 구조체에 반영
                schedule->calendar_id = calendars[selected_cal].calendar_id;
                wcsncpy_s(schedule->title, 64, title, _TRUNCATE);
                wcsncpy_s(schedule->location, 64, location, _TRUNCATE);
                wcsncpy_s(schedule->memo, 256, memo, _TRUNCATE);
                schedule->is_all_day = is_allday;
                schedule->repeat_type = (RepeatType)repeat_type;
                // 날짜/시간 파싱해서 schedule->start_time / end_time 채우는 기존 코드 그대로 유지

                if (!ScheduleDialog_ConfirmSave(schedule)) {
                    // "아니오"를 누르면 다시 편집 화면으로 복귀
                    need_redraw = 1;
                    continue;
                }

                ScheduleLogic_Update(schedule);
                set_cursor_visibility(0);
                return 1;
            }
            else if (Ui_PointInRect(&rect_cancel, mx, my)) {
                // 삭제 확인 팝업
                if (ScheduleDialog_ConfirmDelete(schedule)) {
                    // 사용자가 "예"를 눌렀을 때만 실제 삭제
                    FileIO_DeleteSchedule(schedule->schedule_id);
                    set_cursor_visibility(0);
                    return 1;   // 삭제 완료 후 다이얼로그 닫기
                }
                // "아니오"를 선택하면 다시 편집 화면으로 돌아감
                need_redraw = 1;
            }
        }
        else if (ev.type == UI_INPUT_KEY) {
            wchar_t ch = ev.key;
            if (ch == 27) {
                return 0;
            }
            if (ch == L'\t') {
                focused = (DialogField)(((int)focused + 1) % FIELD_MAX);
                if (focused >= FIELD_SAVE) focused = FIELD_CALENDAR;
                need_redraw = 1;
                continue;
            }
            if (ch == L'\r' || ch == L'\n') {
                if (title[0] == L'\0') {
                    goto_xy(12, 27);
                    wprintf(L"! \xc1\xa6\xb8\xf1\xc0\xbb \xc0韜\xc2\xc7究\xbc\xbf\xe4!                              ");
                    Sleep(1500);
                    need_redraw = 1;
                    continue;
                }
                schedule->calendar_id = calendars[selected_cal].calendar_id;
                wcsncpy_s(schedule->title, 64, title, _TRUNCATE);
                wcsncpy_s(schedule->location, 64, location, _TRUNCATE);
                wcsncpy_s(schedule->memo, 256, memo, _TRUNCATE);
                schedule->is_all_day = is_allday;
                schedule->repeat_type = (RepeatType)repeat_type;
                int yv, mv, dv, hv, minv;
                if (swscanf_s(start_date, L"%d-%d-%d", &yv, &mv, &dv) == 3) {
                    schedule->start_time.tm_year = yv - 1900;
                    schedule->start_time.tm_mon = mv - 1;
                    schedule->start_time.tm_mday = dv;
                }
                if (!is_allday && swscanf_s(start_time, L"%d:%d", &hv, &minv) == 2) {
                    schedule->start_time.tm_hour = hv;
                    schedule->start_time.tm_min = minv;
                }
                if (swscanf_s(end_date, L"%d-%d-%d", &yv, &mv, &dv) == 3) {
                    schedule->end_time.tm_year = yv - 1900;
                    schedule->end_time.tm_mon = mv - 1;
                    schedule->end_time.tm_mday = dv;
                }
                if (!is_allday && swscanf_s(end_time, L"%d:%d", &hv, &minv) == 2) {
                    schedule->end_time.tm_hour = hv;
                    schedule->end_time.tm_min = minv;
                }
                ScheduleLogic_Update(schedule);
                set_cursor_visibility(0);
                return 1;
            }
            if (ch == L'\b') {
                wchar_t* buf = NULL;
                if (focused == FIELD_TITLE) buf = title;
                else if (focused == FIELD_LOCATION) buf = location;
                else if (focused == FIELD_START_DATE) buf = start_date;
                else if (focused == FIELD_START_TIME) buf = start_time;
                else if (focused == FIELD_END_DATE) buf = end_date;
                else if (focused == FIELD_END_TIME) buf = end_time;
                else if (focused == FIELD_MEMO) buf = memo;
                if (buf) {
                    size_t len = wcslen(buf);
                    if (len > 0) buf[len - 1] = L'\0';
                    need_redraw = 1;
                }
            }
            else if (iswprint(ch) || ch == L' ') {
                wchar_t* buf = NULL;
                int max_len = 0;
                if (focused == FIELD_TITLE) { buf = title; max_len = 63; }
                else if (focused == FIELD_LOCATION) { buf = location; max_len = 63; }
                else if (focused == FIELD_START_DATE) { buf = start_date; max_len = 15; }
                else if (focused == FIELD_START_TIME && !is_allday) { buf = start_time; max_len = 7; }
                else if (focused == FIELD_END_DATE) { buf = end_date; max_len = 15; }
                else if (focused == FIELD_END_TIME && !is_allday) { buf = end_time; max_len = 7; }
                else if (focused == FIELD_MEMO) { buf = memo; max_len = 255; }
                if (buf) {
                    size_t len = wcslen(buf);
                    if (len < (size_t)max_len) {
                        buf[len] = ch;
                        buf[len + 1] = L'\0';
                        need_redraw = 1;
                    }
                }
            }
        }
    }
    return 0;
}
 
int ScheduleDialog_ConfirmSave(const Schedule* schedule)
{
    int dialog_w = 60;
    int dialog_h = 9;

    int dialog_x = 2 + (116 - dialog_w) / 2;
    int dialog_y = 1 + (28 - dialog_h) / 2;

    // 테두리 먼저
    draw_box(dialog_x, dialog_y, dialog_w, dialog_h);

    // 내부를 공백으로 채워서 밑의 내용 가리기
    for (int y = dialog_y + 1; y < dialog_y + dialog_h - 1; y++) {
        goto_xy(dialog_x + 1, y);
        for (int x = 0; x < dialog_w - 2; x++) {
            wprintf(L" ");
        }
    }

    // 라인 깨끗하게 한 번 더
    draw_box(dialog_x, dialog_y, dialog_w, dialog_h);

    // 제목 / 메시지
    goto_xy(dialog_x + 2, dialog_y + 1);
    wprintf(L"일정 저장 확인");

    goto_xy(dialog_x + 2, dialog_y + 3);
    wprintf(L"이 일정을 저장하시겠습니까?");

    goto_xy(dialog_x + 2, dialog_y + 4);
    wprintf(L"일정: %ls", schedule->title);

    // 버튼 배치: [예] [아니오]
    int btn_w = 10;
    int btn_h = 3;
    int gap = 4;
    int btn_y = dialog_y + dialog_h - 3;

    int btn_total_w = btn_w * 2 + gap;
    int btn_x_start = dialog_x + (dialog_w - btn_total_w) / 2;

    int btn_yes_x = btn_x_start;
    int btn_no_x = btn_x_start + btn_w + gap;

    // [예] 버튼
    draw_box(btn_yes_x, btn_y, btn_w, btn_h);
    goto_xy(btn_yes_x + 3, btn_y + 1);
    wprintf(L"예");

    // [아니오] 버튼
    draw_box(btn_no_x, btn_y, btn_w, btn_h);
    goto_xy(btn_no_x + 2, btn_y + 1);
    wprintf(L"아니오");

    UiRect rect_yes = { btn_yes_x, btn_y, btn_w, btn_h };
    UiRect rect_no = { btn_no_x,  btn_y, btn_w, btn_h };

    UiInputEvent ev;

    while (1)
    {
        if (!Ui_WaitInput(&ev))
            continue;

        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x;
            int my = ev.pos.y;

            if (Ui_PointInRect(&rect_yes, mx, my)) {
                // 저장 실행
                return 1;
            }
            if (Ui_PointInRect(&rect_no, mx, my)) {
                // 저장 취소
                return 0;
            }
        }

        // 마우스 전용 화면이라 키보드는 무시
    }

    return 0;
}