#ifndef SCHEDULE_DIALOG_H
#define SCHEDULE_DIALOG_H

#include "struct.h"

// 일정 등록 다이얼로그
// calendars: 선택 가능한 캘린더 목록
// cal_count: 캘린더 개수
// default_date: 기본 날짜
// 반환: 1=저장, 0=취소
int ScheduleDialog_Add(const Calendar* calendars, int cal_count,
                       const struct tm* default_date,
                       Schedule* out_schedule);

// 일정 수정 다이얼로그
int ScheduleDialog_Edit(const Calendar* calendars, int cal_count,
                        Schedule* schedule);

// 일정 삭제 확인
int ScheduleDialog_ConfirmDelete(const Schedule* schedule);

#endif
