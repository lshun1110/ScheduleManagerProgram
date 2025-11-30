// stats.c
#include "common.h"
#include "stats.h"
#include "ui_core.h"
#include "color_util.h"
#include "file_io.h"
#include "calendar_mgr.h"

#include <stdlib.h> // for malloc and free

extern User g_current_user;

static void CalculateStats(const Schedule* schedules, int sched_count,
                           const Calendar* calendars, int cal_count,
                           CalendarStats* stats, int* stats_count) {
    *stats_count = 0;
    int total_minutes = 0;
    
    // 캘린더별로 통계 계산
    for (int i = 0; i < cal_count; i++) {
        stats[i].color = calendars[i].color;
        wcsncpy_s(stats[i].calendar_name, 32, calendars[i].name, _TRUNCATE);
        stats[i].count = 0;
        stats[i].total_minutes = 0;
        
        for (int j = 0; j < sched_count; j++) {
            if (schedules[j].calendar_id == calendars[i].calendar_id && !schedules[j].is_deleted) {
                stats[i].count++;
                
                // 시간 계산
                time_t start = mktime((struct tm*)&schedules[j].start_time);
                time_t end = mktime((struct tm*)&schedules[j].end_time);
                int minutes = (int)difftime(end, start) / 60;
                stats[i].total_minutes += minutes;
                total_minutes += minutes;
            }
        }
        (*stats_count)++;
    }
    
    // 비율 계산
    for (int i = 0; i < *stats_count; i++) {
        if (total_minutes > 0) {
            stats[i].percentage = (stats[i].total_minutes * 100.0f) / total_minutes;
        } else {
            stats[i].percentage = 0.0f;
        }
    }
}

void Stats_ShowMonthly(const struct tm* month) {
    Ui_ClearScreen();
    draw_box(0, 0, 119, 29);
    goto_xy(1,1);
    wprintf(L"<- Back");
    
    goto_xy(33, 7);
    wprintf(L"━━━━━━━━━━━━━━━ 월간 통계 (%04d년 %02d월) ━━━━━━━━━━━━━━━",
            month->tm_year + 1900, month->tm_mon + 1);
    
    // 캘린더 로드
// 1) 우선 전체(내 + 공유) 캘린더를 가져온 뒤
// 2) 그 중에서 "owner == 현재 사용자" 인 것만 통계 대상으로 사용
    Calendar calendars[32];
    Calendar all_calendars[32];

    int all_count = CalMgr_GetAllCalendars(g_current_user.user_id,
        all_calendars, 32);

    int cal_count = 0;
    for (int i = 0; i < all_count && cal_count < 32; i++) {
        // owner가 나(g_current_user.user_id)인 캘린더만 통계에 포함
        if (wcscmp(all_calendars[i].user_id, g_current_user.user_id) == 0) {
            calendars[cal_count++] = all_calendars[i];
        }
    }
    
    // Dynamically allocate arrays for schedules to avoid large stack usage
    Schedule* schedules = (Schedule*)malloc(sizeof(Schedule) * 1000);
    int sched_count = FileIO_LoadSchedules(schedules, 1000);

    Schedule* month_schedules = (Schedule*)malloc(sizeof(Schedule) * 1000);
    int month_sched_count = 0;
    for (int i = 0; i < sched_count; i++) {
        if (schedules[i].start_time.tm_year == month->tm_year &&
            schedules[i].start_time.tm_mon == month->tm_mon &&
            !schedules[i].is_deleted) {
            month_schedules[month_sched_count++] = schedules[i];
        }
    }
    
    // Allocate stats array on heap to avoid large stack usage
    CalendarStats* stats = (CalendarStats*)malloc(sizeof(CalendarStats) * 32);
    int stats_count = 0;
    CalculateStats(month_schedules, month_sched_count, calendars, cal_count, stats, &stats_count);
    
    // 테이블 헤더
    goto_xy(33, 10);
    wprintf(L"캘린더              개수      누적시간         비율");
    goto_xy(33, 11);
    for (int i = 0; i < 55; i++) wprintf(L"─");
    
    // 통계 출력
    int y = 12;
    for (int i = 0; i < stats_count; i++) {
        goto_xy(33, y);
        
        wprintf(L"%d.",i+1);
        ResetColor();
        wprintf(L" %-12ls", stats[i].calendar_name);
        
        goto_xy(51, y);
        wprintf(L"%4d개", stats[i].count);
        
        goto_xy(61, y);
        int hours = stats[i].total_minutes / 60;
        int mins = stats[i].total_minutes % 60;
        wprintf(L"%3d시간 %2d분", hours, mins);
        
        goto_xy(79, y);
        wprintf(L"%5.1f%%", stats[i].percentage);
        goto_xy(91, y);
        /* bar graph removed */
ResetColor();
        y += 2;
    }
    
    // 합계
    goto_xy(33, y);
    for (int i = 0; i < 55; i++) wprintf(L"─");
    y++;
    
    int total_count = 0;
    int total_minutes = 0;
    for (int i = 0; i < stats_count; i++) {
        total_count += stats[i].count;
        total_minutes += stats[i].total_minutes;
    }
    
    goto_xy(33, y);
    wprintf(L"합계");
    goto_xy(51, y);
    wprintf(L"%4d개", total_count);
    goto_xy(51, y);
    int total_hours = total_minutes / 60;
    int total_mins = total_minutes % 60;
    wprintf(L"%3d시간 %2d분", total_hours, total_mins);
    goto_xy(79, y);
    wprintf(L"100.0%%");
    
    
    // 입력 대기
    while (1) {
        UiInputEvent ev;
        Ui_WaitInput(&ev);
                if ((ev.type == UI_INPUT_KEY && ev.key == 27) || (ev.type == UI_INPUT_MOUSE_LEFT && ev.pos.y == 1 && ev.pos.x >= 1 && ev.pos.x <= 7)) {
            break;
        }
    }

    // Free dynamically allocated memory
    free(schedules);
    free(month_schedules);
    free(stats);
}

void Stats_ShowWeekly(const struct tm* week_start) {
    Ui_ClearScreen();

    // 월간 통계와 동일한 박스 크기/위치
    draw_box(0, 0, 119, 29);
    goto_xy(1, 1);
    wprintf(L"<- Back");

    struct tm week_end = *week_start;
    week_end.tm_mday += 6;
    mktime(&week_end);

    // 헤더 위치: 월간 통계와 동일 (33,7)
    goto_xy(33, 7);
    wprintf(L"━━━━━━━━ 주간 통계 (%04d-%02d-%02d ~ %04d-%02d-%02d) ━━━━━━━━",
        week_start->tm_year + 1900, week_start->tm_mon + 1, week_start->tm_mday,
        week_end.tm_year + 1900, week_end.tm_mon + 1, week_end.tm_mday);

    // 캘린더 로드
    // 1) 전체(내 + 공유) 캘린더를 가져온 뒤
    // 2) 그 중에서 "owner == 현재 사용자" 인 것만 통계 대상으로 사용 (월간 통계와 동일한 방식)
    Calendar calendars[32];
    Calendar all_calendars[32];

    int all_count = CalMgr_GetAllCalendars(g_current_user.user_id,
        all_calendars, 32);

    int cal_count = 0;
    for (int i = 0; i < all_count && cal_count < 32; i++) {
        if (wcscmp(all_calendars[i].user_id, g_current_user.user_id) == 0) {
            calendars[cal_count++] = all_calendars[i];
        }
    }

    // Dynamically allocate arrays for schedules to avoid large stack usage
    Schedule* schedules = (Schedule*)malloc(sizeof(Schedule) * 1000);
    int sched_count = FileIO_LoadSchedules(schedules, 1000);

    Schedule* week_schedules = (Schedule*)malloc(sizeof(Schedule) * 1000);
    int week_sched_count = 0;
    time_t week_start_t = mktime((struct tm*)week_start);
    time_t week_end_t = mktime(&week_end);

    for (int i = 0; i < sched_count; i++) {
        if (schedules[i].is_deleted) continue;
        time_t sched_t = mktime(&schedules[i].start_time);
        if (sched_t >= week_start_t && sched_t <= week_end_t) {
            week_schedules[week_sched_count++] = schedules[i];
        }
    }

    // Allocate stats array on heap to avoid large stack usage
    CalendarStats* stats = (CalendarStats*)malloc(sizeof(CalendarStats) * 32);
    int stats_count = 0;
    CalculateStats(week_schedules, week_sched_count, calendars, cal_count,
        stats, &stats_count);

    // 테이블 헤더 (월간 통계와 동일 좌표)
    goto_xy(33, 10);
    wprintf(L"캘린더              개수      누적시간         비율");
    goto_xy(33, 11);
    for (int i = 0; i < 55; i++) wprintf(L"─");

    // 통계 출력 (시작 y = 12, 원래대로)
    int y = 12;
    for (int i = 0; i < stats_count; i++) {
        goto_xy(33, y);
        wprintf(L"%d.", i+1);
        ResetColor();
        wprintf(L" %-12ls", stats[i].calendar_name);

        goto_xy(51, y);
        wprintf(L"%4d개", stats[i].count);

        goto_xy(61, y);
        int hours = stats[i].total_minutes / 60;
        int mins = stats[i].total_minutes % 60;
        wprintf(L"%3d시간 %2d분", hours, mins);

        goto_xy(79, y);
        wprintf(L"%5.1f%%", stats[i].percentage);

        goto_xy(91, y);
        // bar graph 생략
        ResetColor();

        y += 2;
    }

    // 합계
    goto_xy(33, y);
    for (int i = 0; i < 55; i++) wprintf(L"─");
    y++;

    int total_count = 0;
    int total_minutes = 0;
    for (int i = 0; i < stats_count; i++) {
        total_count += stats[i].count;
        total_minutes += stats[i].total_minutes;
    }

    goto_xy(33, y);
    wprintf(L"합계");
    goto_xy(51, y);
    wprintf(L"%4d개", total_count);
    goto_xy(61, y);
    int total_hours = total_minutes / 60;
    int total_mins = total_minutes % 60;
    wprintf(L"%3d시간 %2d분", total_hours, total_mins);
    goto_xy(79, y);
    wprintf(L"100.0%%");


    // 입력 대기 (Back 클릭도 월간과 동일)
    while (1) {
        UiInputEvent ev;
        Ui_WaitInput(&ev);
        if ((ev.type == UI_INPUT_KEY && ev.key == 27) ||
            (ev.type == UI_INPUT_MOUSE_LEFT &&
                ev.pos.y == 1 && ev.pos.x >= 1 && ev.pos.x <= 7)) {
            break;
        }
    }

    // Free dynamically allocated memory
    free(schedules);
    free(week_schedules);
    free(stats);
}