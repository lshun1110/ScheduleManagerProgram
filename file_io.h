#ifndef FILE_IO_H
#define FILE_IO_H

#include "struct.h"

#define CALENDAR_COMPACT_DELETE_COUNT 50
#define SCHEDULE_COMPACT_DELETE_COUNT 100

// 사용자
int FileIO_LoadUsers(User* buf, int max_count);
int FileIO_AddUser(const User* user);
int FileIO_UpdateUser(const User* user);
int FileIO_DeleteUser(const wchar_t* user_id);

// 캘린더
int FileIO_LoadCalendars(Calendar* buf, int max_count);
int FileIO_AddCalendar(const Calendar* cal);
int FileIO_UpdateCalendar(const Calendar* cal);
int FileIO_DeleteCalendar(int calendar_id);

// 일정 - 하나는 이름 다른데서 사용
int FileIO_LoadSchedules(Schedule* buf, int max_count);
int FileIO_AddSchedule(const Schedule* sched);
int FileIO_UpdateSchedule(const Schedule* sched);
int FileIO_DeleteSchedule(int schedule_id);

// 공유
int FileIO_LoadShares(Share* buf, int max_count);
int FileIO_AddShare(const Share* share);
int FileIO_DeleteShare(int share_id);
int FileIO_UpdateSharePermission(int share_id, int permission);

#endif
