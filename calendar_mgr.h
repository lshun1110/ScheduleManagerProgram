#ifndef CALENDAR_MGR_H
#define CALENDAR_MGR_H

#include "struct.h"

// 캘린더 관리
int CalMgr_GetAllCalendars(const wchar_t* user_id, Calendar* buf, int max_count);
int CalMgr_AddCalendar(const wchar_t* user_id, const wchar_t* name, int color);
int CalMgr_ToggleCalendar(int calendar_id);
int CalMgr_DeleteCalendar(int calendar_id);

// 활성 캘린더
int CalMgr_GetActiveCalendarIds(const wchar_t* user_id, int* buf, int max_count);

// 일정 조회
int CalMgr_GetSchedulesForDate(const int* calendar_ids, int cal_count,
                                const struct tm* date,
                                Schedule* buf, int max_count);

int CalMgr_GetSchedulesForMonth(const int* calendar_ids, int cal_count,
                                 int year, int month,
                                 Schedule* buf, int max_count);

#endif
