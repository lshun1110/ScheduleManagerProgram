#ifndef SCHEDULE_LOGIC_H
#define SCHEDULE_LOGIC_H

#include "struct.h"

// 모든 일정을 파일에서 로드
int ScheduleLogic_LoadAll(Schedule* buf, int max_count);

// 모든 일정을 파일에 저장
int ScheduleLogic_SaveAll(const Schedule* buf, int count);

// 특정 날짜의 일정들을 가져오기
int ScheduleLogic_GetSchedulesForDate(const struct tm* date,
                                      Schedule* out_buf,
                                      int max_count);

// 새 일정 추가
int ScheduleLogic_Add(const Schedule* sched);

// 일정 수정
int ScheduleLogic_Update(const Schedule* sched);

// 일정 삭제
int ScheduleLogic_Delete(int schedule_id);

#endif // SCHEDULE_LOGIC_H
