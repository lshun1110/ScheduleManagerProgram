// calendar_ui.h
#ifndef CALENDAR_UI_H
#define CALENDAR_UI_H

#include "common.h"
#include "ui_core.h"

// 캘린더 체크박스 정보
typedef struct {
    int calendar_id;
    wchar_t name[32];
    int color;
    int is_checked;
    UiRect rect;
} CalendarCheckbox;

// 뷰 타입
typedef enum {
    VIEW_MONTHLY = 0,
    VIEW_WEEKLY = 1
} ViewType;

// 헤더 그리기 (상단 메뉴)
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
                           ViewType view_type);

// 왼쪽 패널 그리기 (캘린더 목록 + 체크박스)
void CalendarUi_DrawLeftPanel(CalendarCheckbox* calendars, int* calendar_count,
                               UiRect* rect_new_calendar,
                               UiRect* rect_share_mgr);

// 월간 뷰 그리기
void CalendarUi_DrawMonthGrid(const struct tm* current_month,
                              const struct tm* selected_date,
                              const int* active_calendar_ids,
                              int active_count,
                              UiRect rect_cells[6][7]);

// 주간 뷰 그리기
void CalendarUi_DrawWeekGrid(const struct tm* week_start,
                            const struct tm* selected_date,
                            const int* active_calendar_ids,
                            int active_count,
                            UiRect rect_cells[7]);

// 오른쪽 패널 그리기 (일정 목록)
void CalendarUi_DrawRightPanel(const struct tm* selected_date,
                               const int* active_calendar_ids,
                               int active_count,
                               UiRect* rect_new_schedule);

// 연도/월 이동 다이얼로그
int CalendarUi_ShowYearMonthDialog(struct tm* current_month);

#endif // CALENDAR_UI_H
