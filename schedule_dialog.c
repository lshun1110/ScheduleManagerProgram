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
    draw_box(10, 2, 100, 26);
    
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
        wprintf(L"%-66ls", title);
    } else if (focused != FIELD_TITLE) {
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
        wprintf(L"%-66ls", location);
    } else if (focused != FIELD_LOCATION) {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"장소 (선택사항)");
        ResetColor();
    }
    
    // 시작 날짜/시간
    goto_xy(12, 14);
    wprintf(L"시작");
    draw_box(23, 14, 15, 3);
    goto_xy(25, 15);
    if (start_date[0]) {
        wprintf(L"%-11ls", start_date);
    } else {
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
    } else if (start_time[0]) {
        wprintf(L"%-6ls", start_time);
    } else {
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
    } else {
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
    } else if (end_time[0]) {
        wprintf(L"%-6ls", end_time);
    } else {
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
        wprintf(L"%-66ls", memo);
    } else if (focused != FIELD_MEMO) {
        SetColor(COLOR_WHITE - 8, COLOR_BLACK);
        wprintf(L"메모 (선택사항)");
        ResetColor();
    }
    
    // 종일 여부
    goto_xy(12, 21);
    wprintf(is_allday ? L"[v] 종일 일정" : L"[ ] 종일 일정");
    
    // 반복
    goto_xy(40, 21);
    const wchar_t* repeat_names[] = {L"없음", L"매일", L"매주", L"매월"};
    wprintf(L"반복: ◀ %ls ▶", repeat_names[repeat_type]);
    
    // 버튼
    draw_box(35, 23, 15, 3);
    goto_xy(40, 24);
    wprintf(L"저 장");
    
    draw_box(55, 23, 15, 3);
    goto_xy(60, 24);
    wprintf(L"취 소");
    
    goto_xy(12, 27);
    wprintf(L"Tab: 다음 필드 | Enter: 저장 | ESC: 취소");
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
    UiRect rect_cal_left = {55, 6, 2, 1};
    UiRect rect_cal_right = {58, 6, 2, 1};
    UiRect rect_title = {25, 9, 66, 1};
    UiRect rect_location = {25, 12, 66, 1};
    UiRect rect_start_date = {25, 15, 11, 1};
    UiRect rect_start_time = {42, 15, 6, 1};
    UiRect rect_end_date = {60, 15, 11, 1};
    UiRect rect_end_time = {77, 15, 6, 1};
    UiRect rect_memo = {25, 18, 66, 1};
    UiRect rect_allday = {12, 21, 13, 1};
    UiRect rect_repeat_left = {47, 21, 2, 1};
    UiRect rect_repeat_right = {57, 21, 2, 1};
    UiRect rect_save = {35, 23, 15, 3};
    // draw back button
    goto_xy(1,1);
    wprintf(L"<- Back");
    UiRect rect_cancel = {55, 23, 15, 3};
    
    int need_redraw = 1;
    
    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            DrawDialog(calendars, cal_count, selected_cal,
                      title, location, start_date, start_time,
                      end_date, end_time, memo, is_allday, repeat_type, focused);
            goto_xy(1,1);
            wprintf(L"<- Back");
            need_redraw = 0;
        }
        
        // 커서 위치
        if (focused == FIELD_TITLE && title[0]) {
            goto_xy(25 + (int)wcslen(title), 9);
            set_cursor_visibility(1);
        } else if (focused == FIELD_LOCATION && location[0]) {
            goto_xy(25 + (int)wcslen(location), 12);
            set_cursor_visibility(1);
        } else if (focused == FIELD_START_DATE) {
            goto_xy(25 + (int)wcslen(start_date), 15);
            set_cursor_visibility(1);
        } else if (focused == FIELD_START_TIME && !is_allday) {
            goto_xy(42 + (int)wcslen(start_time), 15);
            set_cursor_visibility(1);
        } else if (focused == FIELD_END_DATE) {
            goto_xy(60 + (int)wcslen(end_date), 15);
            set_cursor_visibility(1);
        } else if (focused == FIELD_END_TIME && !is_allday) {
            goto_xy(77 + (int)wcslen(end_time), 15);
            set_cursor_visibility(1);
        } else if (focused == FIELD_MEMO && memo[0]) {
            goto_xy(25 + (int)wcslen(memo), 18);
            set_cursor_visibility(1);
        } else {
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
            } else if (Ui_PointInRect(&rect_cal_right, mx, my)) {
                selected_cal = (selected_cal + 1) % cal_count;
                need_redraw = 1;
            }
            // 필드 클릭
            else if (Ui_PointInRect(&rect_title, mx, my)) {
                focused = FIELD_TITLE;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_location, mx, my)) {
                focused = FIELD_LOCATION;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_start_date, mx, my)) {
                focused = FIELD_START_DATE;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_start_time, mx, my)) {
                focused = FIELD_START_TIME;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_end_date, mx, my)) {
                focused = FIELD_END_DATE;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_end_time, mx, my)) {
                focused = FIELD_END_TIME;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_memo, mx, my)) {
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
            } else if (Ui_PointInRect(&rect_repeat_right, mx, my)) {
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

int ScheduleDialog_ConfirmDelete(const Schedule* schedule) {
    Ui_ClearScreen();
    draw_box(30, 10, 60, 10);
    
    goto_xy(32, 12);
    wprintf(L"정말 삭제하시겠습니까?");
    
    goto_xy(32, 14);
    wprintf(L"제목: %ls", schedule->title);
    
    draw_box(40, 16, 15, 3);
    goto_xy(45, 17);
    wprintf(L"삭제");
    
    draw_box(60, 16, 15, 3);
    goto_xy(65, 17);
    wprintf(L"취소");
    
    UiRect rect_delete = {40, 16, 15, 3};
    // draw back button
    goto_xy(1,1);
    wprintf(L"<- Back");
    UiRect rect_cancel = {60, 16, 15, 3};
    
    while (1) {
        UiInputEvent ev;
        Ui_WaitInput(&ev);
        
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            // back button click
            if (ev.pos.y == 1 && ev.pos.x >= 1 && ev.pos.x <= 7) {
                return 0;
            }
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

// Added ScheduleDialog_Edit implementation
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
    } else {
        start_time[0] = L'\0';
        end_time[0] = L'\0';
    }
    DialogField focused = FIELD_TITLE;

    UiRect rect_cal_left = {55, 6, 2, 1};
    UiRect rect_cal_right = {58, 6, 2, 1};
    UiRect rect_title = {25, 9, 66, 1};
    UiRect rect_location = {25, 12, 66, 1};
    UiRect rect_start_date = {25, 15, 11, 1};
    UiRect rect_start_time = {42, 15, 6, 1};
    UiRect rect_end_date = {60, 15, 11, 1};
    UiRect rect_end_time = {77, 15, 6, 1};
    UiRect rect_memo = {25, 18, 66, 1};
    UiRect rect_allday = {12, 21, 13, 1};
    UiRect rect_repeat_left = {47, 21, 2, 1};
    UiRect rect_repeat_right = {57, 21, 2, 1};
    UiRect rect_save = {35, 23, 15, 3};
    UiRect rect_cancel = {55, 23, 15, 3};

    int need_redraw = 1;
    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            DrawDialog(calendars, cal_count, selected_cal,
                      title, location, start_date, start_time,
                      end_date, end_time, memo, is_allday, repeat_type, focused);
            goto_xy(1,1);
            wprintf(L"<- Back");
            need_redraw = 0;
        }
        // cursor position
        if (focused == FIELD_TITLE && title[0]) {
            goto_xy(25 + (int)wcslen(title), 9);
            set_cursor_visibility(1);
        } else if (focused == FIELD_LOCATION && location[0]) {
            goto_xy(25 + (int)wcslen(location), 12);
            set_cursor_visibility(1);
        } else if (focused == FIELD_START_DATE) {
            goto_xy(25 + (int)wcslen(start_date), 15);
            set_cursor_visibility(1);
        } else if (focused == FIELD_START_TIME && !is_allday) {
            goto_xy(42 + (int)wcslen(start_time), 15);
            set_cursor_visibility(1);
        } else if (focused == FIELD_END_DATE) {
            goto_xy(60 + (int)wcslen(end_date), 15);
            set_cursor_visibility(1);
        } else if (focused == FIELD_END_TIME && !is_allday) {
            goto_xy(77 + (int)wcslen(end_time), 15);
            set_cursor_visibility(1);
        } else if (focused == FIELD_MEMO && memo[0]) {
            goto_xy(25 + (int)wcslen(memo), 18);
            set_cursor_visibility(1);
        } else {
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
            } else if (Ui_PointInRect(&rect_cal_right, mx, my)) {
                selected_cal = (selected_cal + 1) % cal_count;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_title, mx, my)) {
                focused = FIELD_TITLE;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_location, mx, my)) {
                focused = FIELD_LOCATION;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_start_date, mx, my)) {
                focused = FIELD_START_DATE;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_start_time, mx, my)) {
                focused = FIELD_START_TIME;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_end_date, mx, my)) {
                focused = FIELD_END_DATE;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_end_time, mx, my)) {
                focused = FIELD_END_TIME;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_memo, mx, my)) {
                focused = FIELD_MEMO;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_allday, mx, my)) {
                is_allday = !is_allday;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_repeat_left, mx, my)) {
                repeat_type = (repeat_type - 1 + 4) % 4;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_repeat_right, mx, my)) {
                repeat_type = (repeat_type + 1) % 4;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_save, mx, my)) {
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
            } else if (Ui_PointInRect(&rect_cancel, mx, my)) {
                return 0;
            }
        } else if (ev.type == UI_INPUT_KEY) {
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
            } else if (iswprint(ch) || ch == L' ') {
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
