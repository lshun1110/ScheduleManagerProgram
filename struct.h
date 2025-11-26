#ifndef STRUCT_H
#define STRUCT_H

#include <time.h>
#include <wchar.h>

// 사용자 정보
typedef struct {
    wchar_t user_id[32];   // 로그인 ID (고유 키)
    wchar_t password[32];
    wchar_t name[32];
    int     is_deleted;    // 0: 정상, 1: 삭제
} User;

// 캘린더(=카테고리) 정보
typedef struct {
    int     calendar_id;   // 내부 PK (1,2,3,...)
    wchar_t user_id[32];   // 소유자 ID
    wchar_t name[32];      // 캘린더 이름
    int     color;         // 색상 코드
    int     is_active;     // 1: 표시, 0: 숨김
    int     is_deleted;    // 0: 정상, 1: 삭제
} Calendar;

// 반복 타입
typedef enum {
    REPEAT_NONE = 0,
    REPEAT_DAILY = 1,
    REPEAT_WEEKLY = 2,
    REPEAT_MONTHLY = 3
} RepeatType;

// 일정 정보
typedef struct {
    int       schedule_id;
    int       calendar_id;
    wchar_t   title[64];
    wchar_t   location[64];
    wchar_t   memo[256];
    struct tm start_time;
    struct tm end_time;
    int       is_all_day;    // 1: 종일, 0: 일반
    RepeatType repeat_type;  // 반복 유형
    int       is_deleted;    // 0: 정상, 1: 삭제
} Schedule;
// 메모리 상에만 있는 offset / indexing 필드 / 레코드 고정길이
// 공유 캘린더 정보
typedef struct {
    int     share_id;
    int     calendar_id;
    wchar_t owner_id[32];     // 소유자
    wchar_t shared_with[32];  // 공유 받은 사용자
    int     permission;       // 권한 (예: 0:읽기, 1:쓰기 등)
    int     is_deleted;       // 0: 정상, 1: 삭제
} Share;

// 알림 정보
//  → 파일 형식: alarm_id \t schedule_id \t minutes_before \t is_enabled
typedef struct {
    int alarm_id;        // 알림 ID
    int schedule_id;     // 어느 일정의 알림인지
    int minutes_before;  // 일정 시작 몇 분 전인지
    int is_enabled;      // 1: 사용 / 0: 끔
    int is_deleted;      // 0: 정상 / 1: 삭제
} Alarm;

#endif