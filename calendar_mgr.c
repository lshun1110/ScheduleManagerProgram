#include "common.h"
#include "calendar_mgr.h"
#include "file_io.h"
#include "share_mgr.h"

/*
 * Returns all calendars owned by the user as well as calendars shared with the user.
 * Shared calendars are retrieved via ShareMgr_GetSharedCalendars().
 */
int CalMgr_GetAllCalendars(const wchar_t* user_id, Calendar* buf, int max_count) {
    Calendar all_cals[100];
    int total = FileIO_LoadCalendars(all_cals, 100);
    int result_count = 0;

    // First, add calendars owned by the user
    for (int i = 0; i < total && result_count < max_count; i++) {
        if (wcscmp(all_cals[i].user_id, user_id) == 0 && !all_cals[i].is_deleted) {
            buf[result_count++] = all_cals[i];
        }
    }

    // Next, append calendars shared to this user
    if (result_count < max_count) {
        Calendar shared_cals[100];
        int shared_count = ShareMgr_GetSharedCalendars(user_id, shared_cals, 100);
        for (int i = 0; i < shared_count && result_count < max_count; i++) {
            // Avoid duplicates: skip if this calendar already added
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
        
        if (all_scheds[i].start_time.tm_year == date->tm_year &&
            all_scheds[i].start_time.tm_mon == date->tm_mon &&
            all_scheds[i].start_time.tm_mday == date->tm_mday) {
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
