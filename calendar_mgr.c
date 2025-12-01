#include "common.h"
#include "calendar_mgr.h"
#include "file_io.h"
#include "share_mgr.h"

int CalMgr_GetAllCalendars(const wchar_t* user_id, Calendar* buf, int max_count) {
    Calendar all_cals[100];
    int total = FileIO_LoadCalendars(all_cals, 100);
    int result_count = 0;

    for (int i = 0; i < total && result_count < max_count; i++) {
        if (wcscmp(all_cals[i].user_id, user_id) == 0 && !all_cals[i].is_deleted) {
            buf[result_count++] = all_cals[i];
        }
    }

    if (result_count < max_count) {
        Calendar shared_cals[100];
        int shared_count = ShareMgr_GetSharedCalendars(user_id, shared_cals, 100);
        for (int i = 0; i < shared_count && result_count < max_count; i++) {
            int exists = 0;
            for (int j = 0; j < result_count; j++) {
                if (buf[j].calendar_id == shared_cals[i].calendar_id) {
                    exists = 1;
                    break;
                }
            }
            if (!exists) {
                buf[result_count++] = shared_cals[i];
            }
        }
    }
    return result_count;
}

int CalMgr_AddCalendar(const wchar_t* user_id, const wchar_t* name, int color) {
    Calendar cal = {0};
    wcsncpy_s(cal.user_id, 32, user_id, _TRUNCATE);
    wcsncpy_s(cal.name, 32, name, _TRUNCATE);
    cal.color = color;
    cal.is_active = 1;
    cal.is_deleted = 0;
    
    return FileIO_AddCalendar(&cal);
}

int CalMgr_ToggleCalendar(int calendar_id) {
    Calendar calendars[100];
    int count = FileIO_LoadCalendars(calendars, 100);
    
    for (int i = 0; i < count; i++) {
        if (calendars[i].calendar_id == calendar_id) {
            calendars[i].is_active = !calendars[i].is_active;
            return FileIO_UpdateCalendar(&calendars[i]);
        }
    }
    return 0;
}

int CalMgr_DeleteCalendar(int calendar_id) {
    return FileIO_DeleteCalendar(calendar_id);
}

int CalMgr_GetActiveCalendarIds(const wchar_t* user_id, int* buf, int max_count) {
    Calendar calendars[100];
    int cal_count = CalMgr_GetAllCalendars(user_id, calendars, 100);
    
    int result_count = 0;
    for (int i = 0; i < cal_count && result_count < max_count; i++) {
        if (calendars[i].is_active) {
            buf[result_count++] = calendars[i].calendar_id;
        }
    }
    return result_count;
}

int CalMgr_GetSchedulesForDate(const int* calendar_ids, int cal_count,
    const struct tm* date,
    Schedule* buf, int max_count)
{
    Schedule all_scheds[300];
    int total = FileIO_LoadSchedules(all_scheds, 300);

    int result_count = 0;
    for (int i = 0; i < total && result_count < max_count; i++) {
        if (all_scheds[i].is_deleted) continue;

        // 1) 캘린더 필터
        int cal_match = 0;
        for (int j = 0; j < cal_count; j++) {
            if (all_scheds[i].calendar_id == calendar_ids[j]) {
                cal_match = 1;
                break;
            }
        }
        if (!cal_match) continue;

        // 2) 날짜/반복 판정
        int sy = all_scheds[i].start_time.tm_year;
        int sm = all_scheds[i].start_time.tm_mon;
        int sd = all_scheds[i].start_time.tm_mday;

        int dy = date->tm_year;
        int dm = date->tm_mon;
        int dd = date->tm_mday;

        // date >= start_date ?
        int date_after_or_same =
            (dy > sy) ||
            (dy == sy && dm > sm) ||
            (dy == sy && dm == sm && dd >= sd);

        int occurs = 0;
        RepeatType rt = all_scheds[i].repeat_type;

        switch (rt) {
        case REPEAT_NONE:
        default:
            // 기존 방식: 시작 날짜와 완전히 같은 날만
            if (sy == dy && sm == dm && sd == dd) {
                occurs = 1;
            }
            break;

        case REPEAT_DAILY:
            // 시작일 이후(포함) 모든 날
            if (date_after_or_same) {
                occurs = 1;
            }
            break;

        case REPEAT_WEEKLY:
            if (date_after_or_same) {
                // 시작일과 해당 날짜의 일수 차이가 7의 배수인지 검사
                struct tm s = all_scheds[i].start_time;
                struct tm d = *date;
                s.tm_hour = 0; s.tm_min = 0; s.tm_sec = 0;
                d.tm_hour = 0; d.tm_min = 0; d.tm_sec = 0;

                time_t ts = mktime(&s);
                time_t td = mktime(&d);
                if (ts != (time_t)-1 && td != (time_t)-1 && td >= ts) {
                    double diff_sec = difftime(td, ts);
                    int diff_days = (int)(diff_sec / (60 * 60 * 24));
                    if (diff_days % 7 == 0) {
                        occurs = 1;
                    }
                }
            }
            break;

        case REPEAT_MONTHLY:
            // 같은 "일(day-of-month)"이고, 시작일 이후인 달들
            if (dd == sd && date_after_or_same) {
                occurs = 1;
            }
            break;
        }

        if (occurs) {
            buf[result_count++] = all_scheds[i];
        }
    }
    return result_count;
}

int CalMgr_GetSchedulesForMonth(const int* calendar_ids, int cal_count,
                                 int year, int month,
                                 Schedule* buf, int max_count) {
    Schedule all_scheds[300];
    int total = FileIO_LoadSchedules(all_scheds, 300);
    
    int result_count = 0;
    for (int i = 0; i < total && result_count < max_count; i++) {
        if (all_scheds[i].is_deleted) continue;
        
        int cal_match = 0;
        for (int j = 0; j < cal_count; j++) {
            if (all_scheds[i].calendar_id == calendar_ids[j]) {
                cal_match = 1;
                break;
            }
        }
        if (!cal_match) continue;
        
        if (all_scheds[i].start_time.tm_year + 1900 == year &&
            all_scheds[i].start_time.tm_mon + 1 == month) {
            buf[result_count++] = all_scheds[i];
        }
    }
    return result_count;
}
