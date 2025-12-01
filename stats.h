#ifndef STATS_H
#define STATS_H

#include "struct.h"

// 통계 데이터 구조체
typedef struct {
    wchar_t calendar_name[32];
    int color;
    int count;           // 일정 개수
    int total_minutes;   // 누적 시간 (분)
    float percentage;    // 비율
} CalendarStats;

// 월간 통계 표시
void Stats_ShowMonthly(const struct tm* month);

// 주간 통계 표시
void Stats_ShowWeekly(const struct tm* week_start);

#endif
