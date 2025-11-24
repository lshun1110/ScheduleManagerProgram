#include "common.h"
#include "ui_core.h"
#include "app_scene.h"
#include "login_ui.h"


void draw_login_screen()
{
    draw_box(0, 0, 119, 29);
    print_title_segment();

    draw_box(80, 14, 25, 3); // 아이디 박스
    draw_box(80, 17, 25, 3); // 비밀번호 박스

    goto_xy(106, 18); // 비밀번호 보기
    wprintf(L"[ ] view");

    goto_xy(88, 21); // 로그인 버튼
    wprintf(L"[ 로그인 ]");
    goto_xy(87, 23); // 회원가입 버튼
    wprintf(L"[ 회원가입 ]");
}

void print_title_segment()
{
    goto_xy(4, 2);
    wprintf(L"█████  █       ███   █   █  █   █  █████  ████\n");
    goto_xy(4, 3);
    wprintf(L"█   █  █      █   █  ██  █  ██  █  █      █   █\n");
    goto_xy(4, 4);
    wprintf(L"█   █  █      █   █  █ █ █  █ █ █  █      █   █\n");
    goto_xy(4, 5);
    wprintf(L"█████  █      █████  █  ██  █  ██  ████   ████\n");
    goto_xy(4, 6);
    wprintf(L"█      █      █   █  █   █  █   █  █      █ █\n");
    goto_xy(4, 7);
    wprintf(L"█      █      █   █  █   █  █   █  █      █  █\n");
    goto_xy(4, 8);
    wprintf(L"█      █████  █   █  █   █  █   █  █████  █   █\n");
}

// 동적 부분만 업데이트하는 함수
void LoginUi_UpdateFields(const wchar_t* id, const wchar_t* pw, LoginField currentField, int showPassword, int lastLoginFailed)
{
    // ID 필드 업데이트
    goto_xy(82, 15);
    if (id[0] == L'\0' && currentField != FIELD_ID)
    {
        wprintf(L"%-21ls", L"아이디");
    }
    else
    {
        wprintf(L"%-21ls", id);
    }

    // PW 필드 업데이트
    goto_xy(82, 18);
    if (pw[0] == L'\0' && currentField != FIELD_PW)
    {
        wprintf(L"%-21ls", L"비밀번호");
    }
    else
    {
        if (showPassword)
        {
            wprintf(L"%-21ls", pw);
        }
        else
        {
            size_t len = wcslen(pw);
            if (len > 21) len = 21;
            for (size_t i = 0; i < len; ++i) { wprintf(L"*"); }
            for (size_t i = len; i < 21; ++i) { wprintf(L" "); }
        }
    }

    // View 체크박스
    goto_xy(106, 18);
    if (showPassword)
    {
        wprintf(L"[v] view");
    }
    else
    {
        wprintf(L"[ ] view");
    }

    // 로그인 실패 메시지
    goto_xy(30, 13);
    if (lastLoginFailed)
    {
        wprintf(L"로그인에 실패했습니다. ID와 비밀번호를 확인하세요.            ");
    }
    else
    {
        wprintf(L"                                                                ");
    }
}

// 전체 화면 그리기 (처음 한 번만)
void LoginUi_Draw(const wchar_t* id, const wchar_t* pw, LoginField currentField, int showPassword, int lastLoginFailed)
{
    draw_login_screen();
    LoginUi_UpdateFields(id, pw, currentField, showPassword, lastLoginFailed);
}
