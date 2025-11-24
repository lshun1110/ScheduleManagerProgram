// user_menu.h
#ifndef USER_MENU_H
#define USER_MENU_H

#include "struct.h"

// 사용자 메뉴 표시 및 선택
typedef enum {
    USER_MENU_NONE = 0,
    USER_MENU_PROFILE,    // 회원정보 수정
    USER_MENU_SHARE,      // 공유 관리
    USER_MENU_DELETE      // 회원 탈퇴
} UserMenuAction;

UserMenuAction UserMenu_Show(int x, int y);

// 회원정보 수정 다이얼로그
int UserMenu_EditProfile(User* user);

// 회원 탈퇴 확인 다이얼로그
int UserMenu_ConfirmDelete(const wchar_t* user_id);

#endif
