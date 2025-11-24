#include "alarm_logic.h"
#include <stdio.h>
#include <string.h>

#define MAX_ALARMS 256

static Alarm g_alarms[MAX_ALARMS];
static int   g_alarm_count = 0;
static int   g_alarm_loaded = 0;

/*
 * 최초 한 번만 alarms.txt를 읽어서 g_alarms에 채워넣는다.
 * 파일 형식(한 줄):
 *   alarm_id  schedule_id  minutes_before  is_enabled  is_deleted
 * 공백 한 칸으로 구분된 정수 5개를 사용한다.
 * 예: 1 10 30 1 0
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

            // 형식: alarm_id schedule_id minutes_before is_enabled is_deleted
            int scanned = swscanf_s(
                line,
                L"%d %d %d %d %d",
                &id, &sched, &before, &enabled, &deleted
            );

            // 정확히 5개 필드가 있는 경우만 유효한 알림으로 처리
            if (scanned == 5) {
                a.alarm_id = id;
                a.schedule_id = sched;
                a.minutes_before = before;
                a.is_enabled = enabled;
                a.is_deleted = deleted;

                // is_deleted 값이 0/1이 아니면 0으로 보정
                if (a.is_deleted != 0 && a.is_deleted != 1) {
                    a.is_deleted = 0;
                }

                g_alarms[g_alarm_count++] = a;
            }
            // 그 외 형식은 무시
        }

        fclose(fp);
    }

    g_alarm_loaded = 1;
}

/*
 * g_alarms 전체를 alarms.txt로 다시 저장한다.
 */
static int AlarmLogic_SaveAll(void)
{
    FILE* fp = _wfopen(L"alarms.txt", L"w, ccs=UTF-16LE");
    if (!fp) {
        return 0;
    }

    for (int i = 0; i < g_alarm_count; i++) {
        Alarm* a = &g_alarms[i];
        // 형식: alarm_id schedule_id minutes_before is_enabled is_deleted
        fwprintf(
            fp,
            L"%d %d %d %d %d\n",
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
 * 외부에서 알림 목록을 요청할 때 사용하는 함수.
 * 삭제된(is_deleted = 1) 알림은 기본적으로 건너뛴다.
 */
int AlarmLogic_LoadAll(Alarm* buf, int max_count)
{
    if (!buf || max_count <= 0) {
        return 0;
    }

    AlarmLogic_EnsureLoaded();

    int n = 0;
    for (int i = 0; i < g_alarm_count && n < max_count; i++) {
        if (g_alarms[i].is_deleted) {
            continue;  // 삭제된 알림은 기본적으로 숨김
        }
        buf[n++] = g_alarms[i];
    }

    return n;
}

/*
 * 새 알림 추가.
 *  - alarm->alarm_id <= 0 이면 내부에서 새 ID를 할당.
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

    // ID 자동 할당
    if (a.alarm_id <= 0) {
        int max_id = 0;
        for (int i = 0; i < g_alarm_count; i++) {
            if (g_alarms[i].alarm_id > max_id) {
                max_id = g_alarms[i].alarm_id;
            }
        }
        a.alarm_id = max_id + 1;
    }

    // is_deleted 값 보정
    if (a.is_deleted != 0 && a.is_deleted != 1) {
        a.is_deleted = 0;
    }

    g_alarms[g_alarm_count++] = a;

    // 전체를 다시 저장
    return AlarmLogic_SaveAll();
}

/*
 * 알림 하나를 논리 삭제한다.
 *  - alarm_id 에 해당하는 알림을 찾아 is_deleted = 1로 설정하고 저장한다.
 * 반환값 : 성공(1), 실패(0)
 */
int AlarmLogic_Delete(int alarm_id)
{
    if (alarm_id <= 0) {
        return 0;
    }

    AlarmLogic_EnsureLoaded();

    for (int i = 0; i < g_alarm_count; i++) {
        if (g_alarms[i].alarm_id == alarm_id) {
            g_alarms[i].is_deleted = 1;
            return AlarmLogic_SaveAll();
        }
    }

    return 0;
}