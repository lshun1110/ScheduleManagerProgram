#ifndef ALARM_LOGIC_H
#define ALARM_LOGIC_H

#include "struct.h"

/*
 * alarms.txt에서 알림들을 모두 읽어온다.
 *  - buf : 결과를 저장할 배열
 *  - max_count : buf에 저장할 수 있는 최대 개수
 * 반환값 : 실제로 읽어온 알림 개수
 */
int AlarmLogic_LoadAll(Alarm* buf, int max_count);

/*
 * 새 알림 하나를 추가한다.
 *  - alarm->alarm_id <= 0 이면 내부에서 자동으로 ID를 배정한다.
 * 반환값 : 성공(1), 실패(0)
 */
int AlarmLogic_Add(const Alarm* alarm);

#endif // ALARM_LOGIC_H