#include "alarm_logic.h"
#include <stdio.h>
#include <string.h>

#define MAX_ALARMS 256

static Alarm g_alarms[MAX_ALARMS];
static int   g_alarm_count = 0;
static int   g_alarm_loaded = 0;

/*
 * 내부에서 한 번만 alarms.txt를 읽어서 g_alarms에 캐시한다.
 * 파일 형식(한 줄):
 *   alarm_id  schedule_id  minutes_before  is_enabled  is_deleted
 * 탭 또는 공백 구분을 허용한다.
 * 예: 1\t10\t30\t1\t0
 */
static void AlarmLogic_EnsureLoaded(void)
{
    if (g_alarm_loaded) {
        return;
    }

    g_alarm_count = 0;

    FILE* fp = _wfopen(L"alarms.txt", L"r, ccs=UTF-16LE");
    if (fp) {
        wchar_t line[256];

        while (g_alarm_count < MAX_ALARMS && fgetws(line, 256, fp)) {
            Alarm a;
            memset(&a, 0, sizeof(Alarm));

            int id = 0;
            int sched = 0;
            int before = 0;
            int enabled = 0;
            int deleted = 0;

            // 4개 또는 5개 컬럼 모두 처리 (기존 파일 호환)
            int scanned = swscanf_s(
                line,
                L"%d %d %d %d %d",
                &id, &sched, &before, &enabled, &deleted
            );

            if (scanned == 4 || scanned == 5) {
                a.alarm_id = id;
                a.schedule_id = sched;
                a.minutes_before = before;
                a.is_enabled = enabled;
                a.is_deleted = (scanned == 5) ? deleted : 0;

                g_alarms[g_alarm_count++] = a;
            }
            // 그 외 형식은 무시
        }

        fclose(fp);
    }

    g_alarm_loaded = 1;
}

/*
 * g_alarms 전체를 alarms.txt에 다시 저장한다.
 */
static int AlarmLogic_SaveAll(void)
{
    FILE* fp = _wfopen(L"alarms.txt", L"w, ccs=UTF-16LE");
    if (!fp) {
        return 0;
    }

    for (int i = 0; i < g_alarm_count; i++) {
        Alarm* a = &g_alarms[i];
        fwprintf(
            fp,
            L"%d\t%d\t%d\t%d\t%d\n",
            a->alarm_id,
            a->schedule_id,
            a->minutes_before,
            a->is_enabled,
            a->is_deleted
        );
    }

    fclose(fp);
    return 1;
}

/*
 * 외부에서 알림 전부를 요청할 때 사용하는 함수.
 */
int AlarmLogic_LoadAll(Alarm* buf, int max_count)
{
    if (!buf || max_count <= 0) {
        return 0;
    }

    AlarmLogic_EnsureLoaded();

    int n = (g_alarm_count < max_count) ? g_alarm_count : max_count;
    for (int i = 0; i < n; i++) {
        buf[i] = g_alarms[i];
    }

    return n;
}

/*
 * 새 알림 추가.
 *  - alarm->alarm_id <= 0 이면 내부에서 ID 자동 배정.
 *  - is_deleted 값이 0/1이 아니면 0으로 보정.
 */
int AlarmLogic_Add(const Alarm* alarm)
{
    if (!alarm) {
        return 0;
    }

    AlarmLogic_EnsureLoaded();

    if (g_alarm_count >= MAX_ALARMS) {
        return 0;
    }

    Alarm a = *alarm;

    // ID 자동 배정
    if (a.alarm_id <= 0) {
        int max_id = 0;
        for (int i = 0; i < g_alarm_count; i++) {
            if (g_alarms[i].alarm_id > max_id) {
                max_id = g_alarms[i].alarm_id;
            }
        }
        a.alarm_id = max_id + 1;
    }

    // 삭제 플래그 정규화
    if (a.is_deleted != 0 && a.is_deleted != 1) {
        a.is_deleted = 0;
    }

    g_alarms[g_alarm_count++] = a;

    // 전체를 다시 저장해서 형식 통일
    return AlarmLogic_SaveAll();
}