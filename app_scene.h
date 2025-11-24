#ifndef APP_SCENE_H
#define APP_SCENE_H

typedef enum
{
    SCENE_LOGIN = 0,
    SCENE_CALENDAR,
    SCENE_EXIT
} SceneState;

typedef enum
{
    FIELD_ID = 0,     // ID 입력 칸
    FIELD_PW,         // PW 입력 칸
    FIELD_LOGIN,      // [로그인] 버튼
    FIELD_VIEW,       // [ ] view 체크박스
    FIELD_COUNT       // 필드 개수 (순환용)
} LoginField;

// 로그인 루프 내에서 화면 전환용
typedef enum
{
    LOGIN_VIEW_LOGIN = 0,   // 기본 로그인 화면
    LOGIN_VIEW_SIGNUP       // 회원가입 화면
} LoginView;

#endif
