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
// is_deleted 는 상태 변수로 삭제 0 1 로 따지다가 일정 이상 쌓이면 rewrite 한다.
// 공유 캘린더 정보
typedef struct {
    int     share_id;
    int     calendar_id;
    wchar_t owner_id[32];     // 소유자
    wchar_t shared_with[32];  // 공유 받은 사용자
    int     permission;       // 권한 (예: 0:읽기, 1:쓰기 등)
    int     is_deleted;       // 0: 정상, 1: 삭제
} Share;

#endif