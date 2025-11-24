#include "common.h"
#include "signup.h"
#include "ui_core.h"

void draw_signup_screen()
{
    Ui_ClearScreen();
    set_cursor_visibility(0);
    draw_box(0, 0, 119, 29); // 큰 박스

    goto_xy(45,5);
    wprintf(L"Smart Planner에 가입하기");
    goto_xy(49, 7);
    wprintf(L"** 환영합니다 **");

    draw_box(45, 9, 25, 3); // 이름 박스

    draw_box(45, 13, 25, 3); // 아이디 박스

    draw_box(45, 17, 25, 3); // 비밀번호 박스

    goto_xy(71, 14); // 아이디 중복 체크
    wprintf(L"[ 중복 확인 ]");

    goto_xy(71, 18); // 비밀번호 보기
    wprintf(L"[ ] view");

    goto_xy(10, 1); //뒤로 가기
    wprintf(L"[ <- ]");

    goto_xy(52, 21); //회원가입 버튼
    wprintf(L"[ 회원가입 ]");
}
