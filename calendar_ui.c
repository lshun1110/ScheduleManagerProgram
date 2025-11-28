// calendar_ui.c
#include "calendar_ui.h"
#include "schedule_logic.h"
#include "calendar_mgr.h"
#include "file_io.h"
#include "color_util.h"
#include "share_mgr.h" 
#include "struct.h"
#include "app_scene.h"
#include <wchar.h>

extern User g_current_user;

static void put_text(int x, int y, const wchar_t* text)
{
    goto_xy(x, y);
    wprintf(L"%ls", text);
}

void CalendarUi_DrawHeader(const struct tm* current_month,
                           const struct tm* selected_date,
                           UiRect* rect_prev,
                           UiRect* rect_next,
                           UiRect* rect_today,
                           UiRect* rect_logout,
                           UiRect* rect_view_toggle,
                           UiRect* rect_stats,
                           UiRect* rect_search,
                           UiRect* rect_user_menu,
                           ViewType view_type)
{
    draw_box(0, 0, 119, 29);
    
    // 사용자 메뉴 (우측 상단)
    wchar_t userInfo[64];
    swprintf_s(userInfo, 64, L"[%ls ▼]", g_current_user.name);
    int nameLen = (int)wcslen(userInfo);
    
    int menuX = 95;
    put_text(menuX, 1, userInfo);
    rect_user_menu->x = menuX;
    rect_user_menu->y = 1;
    rect_user_menu->w = nameLen;
    rect_user_menu->h = 1;
    
    // 로그아웃 버튼
    int logoutX = 107;
    put_text(logoutX, 1, L"[로그아웃]");
    rect_logout->x = logoutX;
    rect_logout->y = 1;
    rect_logout->w = 10;
    rect_logout->h = 1;
    
    // 제목
    put_text(2, 1, L"SMART PLANNER");

    // 년월 표시 (클릭 가능)
    wchar_t monthStr[32];
    swprintf_s(monthStr, 32, L"[ %04d년 %02d월 ]", 
               current_month->tm_year + 1900, 
               current_month->tm_mon + 1);
    
    put_text(25, 4, L"◀");
    rect_prev->x = 25;
    rect_prev->y = 4;
    rect_prev->w = 2;
    rect_prev->h = 1;
    
    put_text(30, 4, monthStr);
    
    put_text(48, 4, L"▶");
    rect_next->x = 48;
    rect_next->y = 4;
    rect_next->w = 2;
    rect_next->h = 1;
    
    // 버튼들
    put_text(53, 4, L"[오늘]");
    rect_today->x = 53;
    rect_today->y = 4;
    rect_today->w = 6;
    rect_today->h = 1;
    
    put_text(61, 4, view_type == VIEW_MONTHLY ? L"[월간]" : L"[주간]");
    rect_view_toggle->x = 61;
    rect_view_toggle->y = 4;
    rect_view_toggle->w = 6;
    rect_view_toggle->h = 1;
    
    put_text(69, 4, L"[통계]");
    rect_stats->x = 69;
    rect_stats->y = 4;
    rect_stats->w = 6;
    rect_stats->h = 1;
    
    put_text(77, 4, L"[검색]");
    rect_search->x = 77;
    rect_search->y = 4;
    rect_search->w = 6;
    rect_search->h = 1;
}

void CalendarUi_DrawLeftPanel(CalendarCheckbox* calendars, int* calendar_count,
    UiRect* rect_new_calendar,
    UiRect* rect_share_mgr)
{
    int panelX = 2;
    int y = 6;

    // 세로 구분선
    draw_box(20, 5, 1, 23);

    // ===== 내 캘린더 헤더 + [+] 버튼 =====
    goto_xy(panelX, y);
    SetColor(COLOR_YELLOW, COLOR_BLACK);
    wprintf(L"내 캘린더");
    ResetColor();

    // 내 캘린더 [+] 버튼을 오른쪽 고정 위치에 배치
    int newBtnX = panelX + 14;   // 클릭하기 편한 정도로 여유 있게
    goto_xy(newBtnX, y);
    wprintf(L"[+]");

    // 내 캘린더 추가 버튼 클릭 영역
    rect_new_calendar->x = newBtnX;
    rect_new_calendar->y = y;
    rect_new_calendar->w = 3;   // "[+]" 3글자 폭
    rect_new_calendar->h = 1;

    y += 2;

    // 내 캘린더 아래 구분선
    goto_xy(panelX, y);
    for (int i = 0; i < 16; i++) wprintf(L"─");
    y += 1;

    // ===== 내 캘린더 리스트 (owner == g_current_user) =====
    Calendar all_calendars[100];
    int all_count = FileIO_LoadCalendars(all_calendars, 100);

    *calendar_count = 0;

    for (int i = 0; i < all_count && *calendar_count < 32; i++) {
        // 현재 사용자 소유 + 살아있는 캘린더만
        if (wcscmp(all_calendars[i].user_id, g_current_user.user_id) != 0)
            continue;
        if (all_calendars[i].is_deleted)
            continue;

        int idx = *calendar_count;

        calendars[idx].calendar_id = all_calendars[i].calendar_id;
        wcsncpy_s(calendars[idx].name, 32, all_calendars[i].name, _TRUNCATE);
        calendars[idx].color = all_calendars[i].color;
        calendars[idx].is_checked = all_calendars[i].is_active;

        calendars[idx].rect.x = panelX;
        calendars[idx].rect.y = y;
        calendars[idx].rect.w = 16;
        calendars[idx].rect.h = 1;

        goto_xy(panelX, y);
        wprintf(L"%ls ", calendars[idx].is_checked ? L"[v]" : L"[ ]");

        SetColor(calendars[idx].color, COLOR_BLACK);
        wprintf(L"■");
        ResetColor();

        wprintf(L" %.10ls", calendars[idx].name);

        y += 2;
        (*calendar_count)++;
    }

    // 내 캘린더 리스트와 공유 캘린더 헤더 사이 여유 줄 하나
    y += 1;

    // ===== 공유 캘린더 헤더 + [+] 버튼 =====
    goto_xy(panelX, y);
    SetColor(COLOR_CYAN, COLOR_BLACK);
    wprintf(L"공유 캘린더");
    ResetColor();

    // 공유 캘린더 [+] 버튼
    int shareBtnX = panelX + 14;
    goto_xy(shareBtnX, y);
    wprintf(L"[+]");

    // 공유 관리 버튼 클릭 영역
    rect_share_mgr->x = shareBtnX;
    rect_share_mgr->y = y;
    rect_share_mgr->w = 3;  // "[+]"
    rect_share_mgr->h = 1;

    y += 2;

    goto_xy(panelX, y);
    for (int i = 0; i < 16; i++) wprintf(L"─");
    y += 1;

    // ===== 공유받은 캘린더 리스트 =====
    Calendar shared_cals[100];
    int shared_count = ShareMgr_GetSharedCalendars(g_current_user.user_id,
        shared_cals, 100);

    for (int i = 0; i < shared_count && *calendar_count < 32; i++) {
        if (shared_cals[i].is_deleted)
            continue;

        int idx = *calendar_count;

        calendars[idx].calendar_id = shared_cals[i].calendar_id;
        wcsncpy_s(calendars[idx].name, 32, shared_cals[i].name, _TRUNCATE);
        calendars[idx].color = shared_cals[i].color;
        calendars[idx].is_checked = shared_cals[i].is_active;

        calendars[idx].rect.x = panelX;
        calendars[idx].rect.y = y;
        calendars[idx].rect.w = 16;
        calendars[idx].rect.h = 1;

        goto_xy(panelX, y);
        wprintf(L"%ls ", calendars[idx].is_checked ? L"[v]" : L"[ ]");

        SetColor(calendars[idx].color, COLOR_BLACK);
        wprintf(L"■");
        ResetColor();

        wprintf(L" %.10ls", calendars[idx].name);

        y += 2;
        (*calendar_count)++;
    }

    // 예전의 "+ 새 캘린더", "+ 공유 관리" 한 줄짜리 버튼은 삭제
}

void CalendarUi_DrawMonthGrid(const struct tm* current_month,
                              const struct tm* selected_date,
                              const int* active_calendar_ids,
                              int active_count,
                              UiRect rect_cells[6][7])
{
    int startX = 24;
    int startY = 6;
    const int colWidth = 13;
    
    // 요일 헤더
    const wchar_t* weekNames[7] = { 
        L" 월 ", L" 화 ", L" 수 ", L" 목 ", L" 금 ", L" 토 ", L" 일 " 
    };
    
    for (int i = 0; i < 7; ++i) 
    {
        goto_xy(startX + i * 8, startY);
        
        // 토요일: 파란색, 일요일: 빨간색
        if (i == 5) { // 토요일
            SetColor(COLOR_BLUE, COLOR_BLACK);
            wprintf(L"%ls", weekNames[i]);
            ResetColor();
        } else if (i == 6) { // 일요일
            SetColor(COLOR_RED, COLOR_BLACK);
            wprintf(L"%ls", weekNames[i]);
            ResetColor();
        } else {
            wprintf(L"%ls", weekNames[i]);
        }
    }
    
    // 구분선
    goto_xy(startX, startY + 1);
    for (int i = 0; i < 56; i++) wprintf(L"─");
    
    // 달력 계산
    struct tm first_day = *current_month;
    time_t t = mktime(&first_day);
    localtime_s(&first_day, &t);
    
    int wday = (first_day.tm_wday + 6) % 7; // 월요일 시작
    
    struct tm tmp = *current_month;
    tmp.tm_mday = 32;
    mktime(&tmp);
    int days_in_month = 32 - tmp.tm_mday;

    struct tm prev_month = *current_month;
    if (--prev_month.tm_mon < 0) 
    {
        prev_month.tm_mon = 11;
        prev_month.tm_year -= 1;
    }
    prev_month.tm_mday = 32;
    mktime(&prev_month);
    int days_prev_month = 32 - prev_month.tm_mday;

    int dayCounter = 1;
    int nextMonthDay = 1;
    
    for (int row = 0; row < 6; ++row) 
    {
        for (int col = 0; col < 7; ++col) 
        {
            int cellX = startX + col * 8;
            int cellY = startY + 2 + row * 3;
            
            rect_cells[row][col].x = cellX;
            rect_cells[row][col].y = cellY;
            rect_cells[row][col].w = 7;
            rect_cells[row][col].h = 2;
            
            int dateNum;
            struct tm cellDate;
            int isCurrentMonth = 0;
            
            if (row == 0 && col < wday) 
            {
                dateNum = days_prev_month - (wday - col) + 1;
                cellDate = prev_month;
                cellDate.tm_mday = dateNum;
                mktime(&cellDate);
            } 
            else if (dayCounter > days_in_month) 
            {
                dateNum = nextMonthDay++;
                cellDate = *current_month;
                cellDate.tm_mon++;
                if (cellDate.tm_mon > 11) 
                {
                    cellDate.tm_mon = 0;
                    cellDate.tm_year++;
                }
                cellDate.tm_mday = dateNum;
                mktime(&cellDate);
            } 
            else 
            {
                dateNum = dayCounter++;
                cellDate = *current_month;
                cellDate.tm_mday = dateNum;
                mktime(&cellDate);
                isCurrentMonth = 1;
            }
            
            int isSelected = (cellDate.tm_year == selected_date->tm_year &&
                              cellDate.tm_mon == selected_date->tm_mon &&
                              cellDate.tm_mday == selected_date->tm_mday);
            
            time_t now_t = time(NULL);
            struct tm today;
            localtime_s(&today, &now_t);
            int isToday = (cellDate.tm_year == today.tm_year &&
                          cellDate.tm_mon == today.tm_mon &&
                          cellDate.tm_mday == today.tm_mday);
            
            goto_xy(cellX, cellY);
            
            // 색상 결정
            int text_color = COLOR_WHITE;
            if (col == 5) text_color = COLOR_BLUE;      // 토요일
            else if (col == 6) text_color = COLOR_RED;  // 일요일
            
            if (isCurrentMonth) 
            {
                if (isSelected) 
                {
                    // 선택된 날짜: 초록 배경
                    SetColor(COLOR_BLACK, COLOR_GREEN);
                    wprintf(L"[%2d]", dateNum);
                    ResetColor();
                } 
                else if (isToday) 
                {
                    // 오늘: <> 표시
                    SetColor(text_color, COLOR_BLACK);
                    wprintf(L"<%2d>", dateNum);
                    ResetColor();
                } 
                else 
                {
                    SetColor(text_color, COLOR_BLACK);
                    wprintf(L" %2d ", dateNum);
                    ResetColor();
                }
            } 
            else 
            {
                // 다른 월: 어둡게
                SetColor(COLOR_WHITE - 8, COLOR_BLACK);
                wprintf(L" %2d ", dateNum);
                ResetColor();
            }
            
            // 날짜 아래에 캘린더 색상 표시 (활성화된 캘린더만)
            if (isCurrentMonth && active_count > 0) 
            {
                Schedule schedBuf[20];
                int sched_count = CalMgr_GetSchedulesForDate(active_calendar_ids, active_count, &cellDate, schedBuf, 20);
                
                if (sched_count > 0) 
                {
                    goto_xy(cellX, cellY + 1);
                    
                    // 최대 3개 캘린더 색상 표시
                    int color_shown[3] = {-1, -1, -1};
                    int color_count = 0;
                    
                    for (int i = 0; i < sched_count && color_count < 3; i++) {
                        // 이 일정의 캘린더 색상 찾기
                        Calendar calendars[32];
                        int cal_count = CalMgr_GetAllCalendars(g_current_user.user_id, calendars, 32);
                        
                        for (int j = 0; j < cal_count; j++) {
                            if (calendars[j].calendar_id == schedBuf[i].calendar_id) {
                                // 이미 표시한 색상인지 확인
                                int already_shown = 0;
                                for (int k = 0; k < color_count; k++) {
                                    if (color_shown[k] == calendars[j].color) {
                                        already_shown = 1;
                                        break;
                                    }
                                }
                                
                                if (!already_shown) {
                                    SetColor(calendars[j].color, COLOR_BLACK);
                                    wprintf(L"■");
                                    ResetColor();
                                    color_shown[color_count++] = calendars[j].color;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void CalendarUi_DrawWeekGrid(const struct tm* week_start,
                            const struct tm* selected_date,
                            const int* active_calendar_ids,
                            int active_count,
                            UiRect rect_cells[7])
{
    int startX = 24;
    int startY = 6;
    const int colWidth = 13;
    
    goto_xy(startX, startY);
    wprintf(L"주간 뷰 ");
    
    // 기본 레이아웃
    const wchar_t* days[] = {L"월", L"화", L"수", L"목", L"금", L"토", L"일"};
    
    for (int col = 0; col < 7; col++) {
        struct tm day = *week_start;
        day.tm_mday += col;
        mktime(&day);

        int x = startX + col * colWidth;
        // Header: day name and date
        goto_xy(x, startY + 2);
        wprintf(L"%ls %02d", days[col], day.tm_mday);

        // Define cell rect for click handling
        rect_cells[col].x = x;
        rect_cells[col].y = startY + 4;
        rect_cells[col].w = colWidth - 1;
        rect_cells[col].h = 15;

        // Draw schedules for this day
        if (active_count > 0) {
            Schedule sbuf[50];
            int scount = CalMgr_GetSchedulesForDate(active_calendar_ids, active_count, &day, sbuf, 50);
            // Simple insertion sort by start time
            for (int i = 1; i < scount; i++) {
                Schedule key = sbuf[i];
                int j = i - 1;
                while (j >= 0) {
                    time_t t_j = mktime(&sbuf[j].start_time);
                    time_t t_key = mktime(&key.start_time);
                    if (t_j > t_key) {
                        sbuf[j + 1] = sbuf[j];
                        j--;
                    } else break;
                }
                sbuf[j + 1] = key;
            }
            // Draw up to 6 items (fits in 15 lines)
            int yOffset = startY + 4;
            for (int i = 0; i < scount && i < 6; i++) {
                // Find calendar color
                Calendar cals[32];
                int cal_count = CalMgr_GetAllCalendars(g_current_user.user_id, cals, 32);
                int clr = COLOR_WHITE;
                for (int k = 0; k < cal_count; k++) {
                    if (cals[k].calendar_id == sbuf[i].calendar_id) {
                        clr = cals[k].color;
                        break;
                    }
                }
                goto_xy(x, yOffset + i * 2);
                SetColor(clr, COLOR_BLACK);
                wprintf(L"*");
                ResetColor();
                if (sbuf[i].is_all_day) {
                    wprintf(L" %.5ls", sbuf[i].title);
                } else {
                    wprintf(L" %02d:%02d %.3ls",
                            sbuf[i].start_time.tm_hour,
                            sbuf[i].start_time.tm_min,
                            sbuf[i].title);
                }
            }
            // Indicate more items
            if (scount > 6) {
                goto_xy(x, yOffset + 12);
                wprintf(L"...%d", scount - 6);
            }
        }
    }
}

void CalendarUi_DrawRightPanel(const struct tm* selected_date,
                               const int* active_calendar_ids,
                               int active_count,
                               UiRect* rect_new_schedule)
{
    int panelX = 82;
    int y = 6;
    
    // 구분선
    draw_box(80, 5, 1, 23);
    
    // 날짜 표시
    wchar_t dateStr[64];
    const wchar_t* dayNames[7] = { 
        L"일", L"월", L"화", L"수", L"목", L"금", L"토" 
    };
    
    swprintf_s(dateStr, 64, L"%04d년 %02d월 %02d일 (%ls)", 
               selected_date->tm_year + 1900,
               selected_date->tm_mon + 1, 
               selected_date->tm_mday,
               dayNames[selected_date->tm_wday]);
    
    goto_xy(panelX, y);
    wprintf(L"%.34ls", dateStr);
    y += 2;
    
    goto_xy(panelX, y);
    for (int i = 0; i < 36; i++) wprintf(L"─");
    y += 2;
    
    // 일정 목록
    Schedule schedBuf[50];
    int count = CalMgr_GetSchedulesForDate(active_calendar_ids, active_count, selected_date, schedBuf, 50);
    
    if (count == 0) 
    {
        goto_xy(panelX, y);
        wprintf(L"일정이 없습니다.");
        y += 2;
    } 
    else 
    {
        for (int i = 0; i < count && i < 12; ++i) 
        {
            // 캘린더 색상 찾기
            Calendar calendars[32];
            int cal_count = CalMgr_GetAllCalendars(g_current_user.user_id, calendars, 32);
            int color = COLOR_WHITE;
            
            for (int j = 0; j < cal_count; j++) {
                if (calendars[j].calendar_id == schedBuf[i].calendar_id) {
                    color = calendars[j].color;
                    break;
                }
            }
            
            goto_xy(panelX, y);
            SetColor(color, COLOR_BLACK);
            wprintf(L"■");
            ResetColor();
            
            if (schedBuf[i].is_all_day) 
            {
                wprintf(L" %.31ls", schedBuf[i].title);
            } 
            else 
            {
                wprintf(L" %02d:%02d %.26ls",
                        schedBuf[i].start_time.tm_hour,
                        schedBuf[i].start_time.tm_min,
                        schedBuf[i].title);
            }
            
            y++;
        }
        
        if (count > 12) 
        {
            y++;
            goto_xy(panelX, y);
            wchar_t moreLine[32];
            swprintf_s(moreLine, 32, L"외 %d개 더...", count - 12);
            wprintf(L"%ls", moreLine);
        }
        y += 2;
    }
    
    // 새 일정 버튼
    y = 26;
    goto_xy(panelX + 6, y);
    wprintf(L"[ 새 일정 등록 ]");
    rect_new_schedule->x = panelX + 6;
    rect_new_schedule->y = y;
    rect_new_schedule->w = 16;
    rect_new_schedule->h = 1;
}

int CalendarUi_ShowYearMonthDialog(struct tm* current_month)
{
    int year = current_month->tm_year + 1900;
    int month = current_month->tm_mon + 1;
    int focused = 0; // 0: year, 1: month
    int need_redraw = 1;

    wchar_t year_str[8] = L"";
    wchar_t month_str[4] = L"";
    swprintf_s(year_str, 8, L"%d", year);
    swprintf_s(month_str, 4, L"%d", month);

    // ── 팝업 위치/크기 (120x30 기준 중앙 정렬) ─────────────────
    int console_w = 120;
    int console_h = 30;

    int dialog_w = 50;
    int dialog_h = 14;

    int dialog_x = (console_w - dialog_w) / 2;
    int dialog_y = (console_h - dialog_h) / 2;

    // 연/월 입력 박스 공통 설정
    int field_box_w = 14;
    int field_box_h = 3;

    const wchar_t* year_label = L"연도:";
    const wchar_t* month_label = L"월:";

    int year_label_len = (int)wcslen(year_label);
    int month_label_len = (int)wcslen(month_label);
    int max_label_len = (year_label_len > month_label_len) ? year_label_len : month_label_len;

    // "라벨 + 공백 1칸 + 박스" 전체 폭을 기준으로 가운데 정렬
    int total_line_w = max_label_len + 1 + field_box_w;
    int line_start_x = dialog_x + (dialog_w - total_line_w) / 2;

    int label_x = line_start_x;
    int field_x = line_start_x + max_label_len + 1; // 라벨 바로 오른쪽부터 박스
    int field_text_x = field_x + 2;                      // 박스 안쪽 텍스트 시작

    int year_label_y = dialog_y + 3;
    int month_label_y = year_label_y + 4;
    int year_text_y = year_label_y;
    int month_text_y = month_label_y;

    // 버튼 (가운데 정렬)
    int btn_w = 12;
    int btn_h = 3;
    int gap = 4;

    int btn_area_w = btn_w * 2 + gap;
    int btn_x_start = dialog_x + (dialog_w - btn_area_w) / 2;
    int btn_y = dialog_y + dialog_h - 4;

    UiRect rect_year = { field_text_x, year_text_y, 10, 1 };
    UiRect rect_month = { field_text_x, month_text_y, 10, 1 };
    UiRect rect_ok = { btn_x_start,             btn_y, btn_w, btn_h };
    UiRect rect_cancel = { btn_x_start + btn_w + gap, btn_y, btn_w, btn_h };
    UiRect rect_dialog = { dialog_x, dialog_y, dialog_w, dialog_h };

    while (1) {
        if (need_redraw) {
            // 배경은 그대로 두고, 팝업만 그림자 + 박스
            for (int i = 0; i < dialog_h; i++) {
                goto_xy(dialog_x + 2, dialog_y + 1 + i);
                SetColor(COLOR_WHITE - 8, COLOR_BLACK);
                for (int j = 0; j < dialog_w; j++) {
                    wprintf(L" ");
                }
                ResetColor();
            }

            // 팝업 박스
            draw_box(dialog_x, dialog_y, dialog_w, dialog_h);

            // 제목 (가운데 정렬)
            const wchar_t* title = L"날짜 이동";
            int title_len = (int)wcslen(title);
            int title_x = dialog_x + (dialog_w - title_len) / 2;
            goto_xy(title_x, dialog_y + 1);
            wprintf(L"%ls", title);

            // 연도 라벨 + 박스 (한 줄 전체가 팝업 가운데)
            goto_xy(label_x, year_label_y);
            wprintf(L"%ls", year_label);
            draw_box(field_x, year_label_y - 1, field_box_w, field_box_h);
            goto_xy(field_text_x, year_text_y);
            wprintf(L"%-10ls", year_str);

            // 월 라벨 + 박스 (연도와 같은 정렬)
            goto_xy(label_x, month_label_y);
            wprintf(L"%ls", month_label);
            draw_box(field_x, month_label_y - 1, field_box_w, field_box_h);
            goto_xy(field_text_x, month_text_y);
            wprintf(L"%-10ls", month_str);

            // 버튼 (가운데 정렬)
            draw_box(rect_ok.x, rect_ok.y, rect_ok.w, rect_ok.h);
            goto_xy(rect_ok.x + 3, rect_ok.y + 1);
            wprintf(L"이 동");

            draw_box(rect_cancel.x, rect_cancel.y, rect_cancel.w, rect_cancel.h);
            goto_xy(rect_cancel.x + 3, rect_cancel.y + 1);
            wprintf(L"취 소");


            need_redraw = 0;
        }

        // 커서 위치
        if (focused == 0) {
            goto_xy(rect_year.x + (int)wcslen(year_str), rect_year.y);
            set_cursor_visibility(1);
        }
        else {
            goto_xy(rect_month.x + (int)wcslen(month_str), rect_month.y);
            set_cursor_visibility(1);
        }

        UiInputEvent ev;
        Ui_WaitInput(&ev);

        // ── 마우스 입력 ─────────────────
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x;
            int my = ev.pos.y;

            if (!Ui_PointInRect(&rect_dialog, mx, my)) {
                continue;
            }

            if (Ui_PointInRect(&rect_year, mx, my)) {
                focused = 0;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_month, mx, my)) {
                focused = 1;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_ok, mx, my)) {
                int new_year = _wtoi(year_str);
                int new_month = _wtoi(month_str);

                if (new_year >= 1900 && new_year <= 2100 &&
                    new_month >= 1 && new_month <= 12) {
                    current_month->tm_year = new_year - 1900;
                    current_month->tm_mon = new_month - 1;
                    current_month->tm_mday = 1;
                    mktime(current_month);
                    set_cursor_visibility(0);
                    return 1;
                }
                else {
                    goto_xy(dialog_x + 3, dialog_y + dialog_h - 2);
                    wprintf(L"! 올바른 연도와 월을 입력하세요!       ");
                    Sleep(1500);
                    need_redraw = 1;
                }
            }
            else if (Ui_PointInRect(&rect_cancel, mx, my)) {
                set_cursor_visibility(0);
                return 0;
            }
        }
        // ── 키보드 입력 ─────────────────
        else if (ev.type == UI_INPUT_KEY) {
            wchar_t ch = ev.key;

            if (ch == 27) { // ESC
                set_cursor_visibility(0);
                return 0;
            }

            if (ch == L'\t') {
                focused = (focused + 1) % 2;
                need_redraw = 1;
                continue;
            }

            if (ch == L'\r' || ch == L'\n') { // Enter
                int new_year = _wtoi(year_str);
                int new_month = _wtoi(month_str);

                if (new_year >= 1900 && new_year <= 2100 &&
                    new_month >= 1 && new_month <= 12) {
                    current_month->tm_year = new_year - 1900;
                    current_month->tm_mon = new_month - 1;
                    current_month->tm_mday = 1;
                    mktime(current_month);
                    set_cursor_visibility(0);
                    return 1;
                }
                else {
                    goto_xy(dialog_x + 3, dialog_y + dialog_h - 2);
                    wprintf(L"! 올바른 연도와 월을 입력하세요!       ");
                    Sleep(1500);
                    need_redraw = 1;
                }
                continue;
            }

            // 숫자 입력/삭제
            wchar_t* buf = (focused == 0) ? year_str : month_str;
            int      max_len = (focused == 0) ? 7 : 3;

            if (ch == L'\b') {
                size_t len = wcslen(buf);
                if (len > 0) {
                    buf[len - 1] = L'\0';
                    need_redraw = 1;
                }
            }
            else if (ch >= L'0' && ch <= L'9') {
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