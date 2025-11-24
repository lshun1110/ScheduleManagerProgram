#include "common.h"
#include "app_scene.h"
#include "ui_core.h"
#include "login_ui.h"
#include "login_logic.h"
#include "login_loop.h"
#include "signup.h"
#include "signup_logic.h"

extern User g_current_user;

/// su_lastMsg 의미:
/// 0: 메시지 없음
/// 1: 실패 - 중복 ID 또는 잘못된 입력
/// 2: 성공 - 사용 가능한 ID
static void UpdateSignupFields(const wchar_t* su_name, const wchar_t* su_id, const wchar_t* su_pw,
    int su_currentField, int su_showPassword, int su_lastMsg)
{
    // Name
    goto_xy(47, 10);
    if (su_name[0] == L'\0' && su_currentField != 0) {
        wprintf(L"%-21ls", L"Name");
    }
    else {
        wprintf(L"%-21ls", su_name);
    }

    // User ID
    goto_xy(47, 14);
    if (su_id[0] == L'\0' && su_currentField != 1) {
        wprintf(L"%-21ls", L"User ID");
    }
    else {
        wprintf(L"%-21ls", su_id);
    }

    // Password
    goto_xy(47, 18);
    if (su_pw[0] == L'\0' && su_currentField != 2) {
        wprintf(L"%-21ls", L"Password");
    }
    else if (su_showPassword) {
        wprintf(L"%-21ls", su_pw);
    }
    else {
        size_t len = wcslen(su_pw);
        if (len > 21) len = 21;
        for (size_t i = 0; i < len; ++i) wprintf(L"*");
        for (size_t i = len; i < 21; ++i) wprintf(L" ");
    }

    // [ ] view 버튼
    goto_xy(71, 18);
    wprintf(su_showPassword ? L"[X] view" : L"[ ] view");

    // 메시지 영역
    goto_xy(45, 23);
    if (su_lastMsg == 1) {
        wprintf(L"회원가입 실패 : 중복 ID 또는 잘못된 입력입니다.            ");
    }
    else if (su_lastMsg == 2) {
        wprintf(L"사용 가능한 ID입니다.                                 ");
    }
    else {
        wprintf(L"                                                ");
    }
}

SceneState Login_Loop(void)
{
    wchar_t id[32] = L"";
    wchar_t pw[32] = L"";

    // 시작 시: 로그인 화면, 포커스는 로그인 버튼(필드 입력 X)
    LoginField currentField = FIELD_LOGIN;
    int showPassword = 0;
    int lastLoginFailed = 0;
    DWORD failTick = 0;

    LoginView view = LOGIN_VIEW_LOGIN;  // LOGIN / SIGNUP
    int login_screen_drawn = 0;

    // 회원가입 화면용 상태
    wchar_t su_name[32] = L"", su_id[32] = L"", su_pw[32] = L"";
    int su_currentField = -1;     // -1: 아무 필드도 선택 안 됨
    int su_showPassword = 0;
    int su_lastMsg = 0;           // 0: 없음, 1: 실패, 2: 성공
    DWORD su_msgTick = 0;
    int su_screen_drawn = 0;
    int su_id_checked_ok = 0;     // 1 이면 최신 ID에 대해 중복 확인 통과

    UiRect rect_id_box = (UiRect){ 80, 14, 25, 3 };
    UiRect rect_pw_box = (UiRect){ 80, 17, 25, 3 };
    UiRect rect_view_box = (UiRect){ 106, 18, 8,  1 };
    UiRect rect_btn_login = (UiRect){ 88, 21, 11, 1 };
    UiRect rect_btn_signup = (UiRect){ 87, 23, 13, 1 };

    UiRect su_rect_name_box = (UiRect){ 45,  9, 25, 3 };
    UiRect su_rect_id_box = (UiRect){ 45, 13, 25, 3 };
    UiRect su_rect_pw_box = (UiRect){ 45, 17, 25, 3 };
    UiRect su_rect_dup_box = (UiRect){ 71, 14, 12, 1 };  // [ 중복 확인 ]
    UiRect su_rect_view_box = (UiRect){ 71, 18,  8, 1 };  // [ ] view
    UiRect su_rect_btn_back = (UiRect){ 10,  1,  8, 1 };  // [ 뒤로 ]
    UiRect su_rect_btn_signup = (UiRect){ 52, 21, 20, 1 };  // [ 회원가입 완료 ]

    while (1)
    {
        // 로그인 실패 메시지 3초 유지
        if (lastLoginFailed && GetTickCount() - failTick >= 3000) {
            lastLoginFailed = 0;
            if (view == LOGIN_VIEW_LOGIN && login_screen_drawn) {
                LoginUi_UpdateFields(id, pw, currentField, showPassword, lastLoginFailed);
            }
        }

        // 회원가입 메시지(성공/실패 모두) 3초 유지
        if (su_lastMsg && GetTickCount() - su_msgTick >= 3000) {
            su_lastMsg = 0;
            if (view == LOGIN_VIEW_SIGNUP && su_screen_drawn) {
                UpdateSignupFields(su_name, su_id, su_pw, su_currentField, su_showPassword, su_lastMsg);
            }
        }

        // 화면 그리기
        if (view == LOGIN_VIEW_LOGIN) {
            if (!login_screen_drawn) {
                Ui_ClearScreen();
                LoginUi_Draw(id, pw, currentField, showPassword, lastLoginFailed);
                login_screen_drawn = 1;
            }
            else {
                LoginUi_UpdateFields(id, pw, currentField, showPassword, lastLoginFailed);
            }
        }
        else { // SIGNUP
            if (!su_screen_drawn) {
                Ui_ClearScreen();
                draw_signup_screen();
                UpdateSignupFields(su_name, su_id, su_pw, su_currentField, su_showPassword, su_lastMsg);
                su_screen_drawn = 1;
            }
            else {
                UpdateSignupFields(su_name, su_id, su_pw, su_currentField, su_showPassword, su_lastMsg);
            }
        }

        // 커서 위치
        if (view == LOGIN_VIEW_LOGIN) {
            if (currentField == FIELD_ID) {
                goto_xy(82 + (int)wcslen(id), 15);
                set_cursor_visibility(1);
            }
            else if (currentField == FIELD_PW) {
                goto_xy(82 + (int)wcslen(pw), 18);
                set_cursor_visibility(1);
            }
            else {
                set_cursor_visibility(0);
            }
        }
        else { // SIGNUP
            if (su_currentField == 0) {
                goto_xy(47 + (int)wcslen(su_name), 10);
                set_cursor_visibility(1);
            }
            else if (su_currentField == 1) {
                goto_xy(47 + (int)wcslen(su_id), 14);
                set_cursor_visibility(1);
            }
            else if (su_currentField == 2) {
                goto_xy(47 + (int)wcslen(su_pw), 18);
                set_cursor_visibility(1);
            }
            else {
                set_cursor_visibility(0);
            }
        }

        UiInputEvent ev;
        Ui_WaitInput(&ev);

        // ===================== 마우스 입력 =====================
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x;
            int my = ev.pos.y;

            if (view == LOGIN_VIEW_LOGIN) {
                if (Ui_PointInRect(&rect_id_box, mx, my)) {
                    currentField = FIELD_ID;
                }
                else if (Ui_PointInRect(&rect_pw_box, mx, my)) {
                    currentField = FIELD_PW;
                }
                else if (Ui_PointInRect(&rect_view_box, mx, my)) {
                    showPassword = !showPassword;
                    currentField = FIELD_VIEW;
                }
                else if (Ui_PointInRect(&rect_btn_login, mx, my)) {
                    if (Login_Auth(id, pw, &g_current_user)) {
                        set_cursor_visibility(0);
                        return SCENE_CALENDAR;
                    }
                    lastLoginFailed = 1;
                    failTick = GetTickCount();
                }
                else if (Ui_PointInRect(&rect_btn_signup, mx, my)) {
                    // 회원가입 화면 진입
                    view = LOGIN_VIEW_SIGNUP;
                    su_currentField = -1; // 처음엔 아무 필드도 선택 안 함
                    su_lastMsg = 0;
                    su_id_checked_ok = 0;
                    login_screen_drawn = 0;
                    su_screen_drawn = 0;
                }
            }
            else {
                // 회원가입 화면
                if (Ui_PointInRect(&su_rect_name_box, mx, my)) {
                    su_currentField = 0;
                }
                else if (Ui_PointInRect(&su_rect_id_box, mx, my)) {
                    su_currentField = 1;
                }
                else if (Ui_PointInRect(&su_rect_pw_box, mx, my)) {
                    su_currentField = 2;
                }
                else if (Ui_PointInRect(&su_rect_dup_box, mx, my)) {
                    // [ 중복 확인 ] 버튼
                    if (Signup_CheckDuplicateId(su_id)) {
                        su_lastMsg = 2;            // 사용 가능
                        su_msgTick = GetTickCount();
                        su_id_checked_ok = 1;
                    }
                    else {
                        su_lastMsg = 1;            // 중복/잘못된 ID
                        su_msgTick = GetTickCount();
                        su_id_checked_ok = 0;
                    }
                    UpdateSignupFields(su_name, su_id, su_pw, su_currentField, su_showPassword, su_lastMsg);
                }
                else if (Ui_PointInRect(&su_rect_view_box, mx, my)) {
                    su_showPassword = !su_showPassword;
                }
                else if (Ui_PointInRect(&su_rect_btn_back, mx, my)) {
                    // 로그인 화면으로 돌아가기 (자동 입력 없음, 포커스는 LOGIN 버튼)
                    view = LOGIN_VIEW_LOGIN;
                    currentField = FIELD_LOGIN;
                    login_screen_drawn = 0;
                    su_screen_drawn = 0;
                }
                else if (Ui_PointInRect(&su_rect_btn_signup, mx, my)) {
                    // [ 회원가입 완료 ] 버튼
                    if (su_id_checked_ok && Signup_CreateUser(su_id, su_pw, su_name)) {
                        // 회원가입 성공: 로그인 화면으로, ID/PW 자동 입력 X
                        id[0] = L'\0';
                        pw[0] = L'\0';
                        view = LOGIN_VIEW_LOGIN;
                        currentField = FIELD_LOGIN;
                        su_name[0] = su_id[0] = su_pw[0] = L'\0';
                        su_id_checked_ok = 0;
                        su_lastMsg = 0;
                        login_screen_drawn = 0;
                        su_screen_drawn = 0;
                    }
                    else {
                        su_lastMsg = 1;            // 중복확인 안 했거나, 생성 실패
                        su_msgTick = GetTickCount();
                    }
                }
            }
            continue;
        }

        // ===================== 키보드 입력 =====================
        if (ev.type == UI_INPUT_KEY) {
            wchar_t ch = ev.key;

            // ESC 처리
            if (ch == 27) {
                if (view == LOGIN_VIEW_SIGNUP) {
                    // 회원가입 화면에서 ESC → 로그인 화면으로
                    view = LOGIN_VIEW_LOGIN;
                    currentField = FIELD_LOGIN;
                    login_screen_drawn = 0;
                    su_screen_drawn = 0;
                }
                else {
                    set_cursor_visibility(0);
                    return SCENE_EXIT;
                }
                continue;
            }

            if (view == LOGIN_VIEW_LOGIN) {
                // 로그인 화면 키 입력
                if (ch == L'\t') {
                    currentField = (LoginField)(((int)currentField + 1) % FIELD_COUNT);
                }
                else if (ch == L'\b') {
                    wchar_t* buf = NULL;
                    if (currentField == FIELD_ID) buf = id;
                    else if (currentField == FIELD_PW) buf = pw;
                    if (buf) {
                        size_t len = wcslen(buf);
                        if (len > 0) buf[len - 1] = L'\0';
                    }
                }
                else if (ch == L'\r' || ch == L'\n') {
                    if (Login_Auth(id, pw, &g_current_user)) {
                        set_cursor_visibility(0);
                        return SCENE_CALENDAR;
                    }
                    lastLoginFailed = 1;
                    failTick = GetTickCount();
                }
                else if (iswprint(ch) || ch == L' ') {
                    if (ch == L' ' && currentField == FIELD_VIEW) {
                        showPassword = !showPassword;
                    }
                    else {
                        wchar_t* buf = NULL;
                        if (currentField == FIELD_ID) buf = id;
                        else if (currentField == FIELD_PW) buf = pw;
                        if (buf) {
                            size_t len = wcslen(buf);
                            if (len < 31) {
                                buf[len] = ch;
                                buf[len + 1] = L'\0';
                            }
                        }
                    }
                }
            }
            else {
                // 회원가입 화면 키 입력
                if (ch == L'\t') {
                    // Name → ID → PW 순환
                    su_currentField = (su_currentField + 1) % 3;
                }
                else if (ch == L'\b') {
                    wchar_t* buf = (su_currentField == 0) ? su_name :
                        (su_currentField == 1) ? su_id : su_pw;
                    size_t len = wcslen(buf);
                    if (len > 0) buf[len - 1] = L'\0';
                    if (su_currentField == 1) {
                        // ID 수정되면 중복 확인 상태 초기화
                        su_id_checked_ok = 0;
                    }
                }
                else if (ch == L'\r' || ch == L'\n') {
                    // Enter로도 회원가입 시도 (버튼 클릭과 동일한 조건)
                    if (su_id_checked_ok && Signup_CreateUser(su_id, su_pw, su_name)) {
                        id[0] = L'\0';
                        pw[0] = L'\0';
                        view = LOGIN_VIEW_LOGIN;
                        currentField = FIELD_LOGIN;
                        su_name[0] = su_id[0] = su_pw[0] = L'\0';
                        su_id_checked_ok = 0;
                        su_lastMsg = 0;
                        login_screen_drawn = 0;
                        su_screen_drawn = 0;
                    }
                    else {
                        su_lastMsg = 1;
                        su_msgTick = GetTickCount();
                    }
                }
                else if (iswprint(ch) || ch == L' ') {
                    wchar_t* buf = (su_currentField == 0) ? su_name :
                        (su_currentField == 1) ? su_id : su_pw;
                    size_t len = wcslen(buf);
                    if (len < 31) {
                        buf[len] = ch;
                        buf[len + 1] = L'\0';
                    }
                    if (su_currentField == 1) {
                        // ID 입력 중이면 항상 다시 중복 확인 필요
                        su_id_checked_ok = 0;
                    }
                }
            }
        }
    }

    return SCENE_EXIT;
}