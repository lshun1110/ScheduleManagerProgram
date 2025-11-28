// user_menu.c
#include "common.h"
#include "user_menu.h"
#include "ui_core.h"
#include "color_util.h"
#include "file_io.h"

UserMenuAction UserMenu_Show(int x, int y) {
    // 드롭다운 메뉴 박스
    draw_box(x, y, 20, 8);
    
    goto_xy(x + 2, y + 1);
    wprintf(L"회원정보 수정");
    goto_xy(x + 2, y + 2);
    wprintf(L"              ");
    goto_xy(x + 2, y + 3);
    wprintf(L"공유 관리");
    goto_xy(x + 2, y + 4);
    wprintf(L"              ");
    goto_xy(x + 2, y + 5);
    wprintf(L"회원 탈퇴");
    goto_xy(x + 1, y + 6);
    wprintf(L"                  ");
    goto_xy(x + 2, y + 7);
    wprintf(L"(ESC: 닫기)");
    
    UiRect rect_profile = {x + 2, y + 1, 16, 1};
    UiRect rect_share = {x + 2, y + 3, 16, 1};
    UiRect rect_delete = {x + 2, y + 5, 16, 1};
    
    while (1) {
        UiInputEvent ev;
        Ui_WaitInput(&ev);
        
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x, my = ev.pos.y;
            
            if (Ui_PointInRect(&rect_profile, mx, my)) {
                return USER_MENU_PROFILE;
            } else if (Ui_PointInRect(&rect_share, mx, my)) {
                return USER_MENU_SHARE;
            } else if (Ui_PointInRect(&rect_delete, mx, my)) {
                return USER_MENU_DELETE;
            }
        } else if (ev.type == UI_INPUT_KEY && ev.key == 27) {
            return USER_MENU_NONE;
        }
    }
}

int UserMenu_EditProfile(User* user) {
    wchar_t new_password[32] = L"";
    wchar_t new_name[32] = L"";
    int focused = 0; // 0: password, 1: name
    
    wcsncpy_s(new_password, 32, user->password, _TRUNCATE);
    wcsncpy_s(new_name, 32, user->name, _TRUNCATE);
    
    UiRect rect_password = {35, 12, 40, 1};
    UiRect rect_name = {35, 16, 40, 1};
    UiRect rect_save = {35, 20, 15, 3};
    UiRect rect_cancel = {55, 20, 15, 3};
    
    int need_redraw = 1;
    
    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            draw_box(20, 8, 70, 18);
            
            goto_xy(22, 9);
            wprintf(L"━━━━━━━━━━━━ 회원정보 수정 ━━━━━━━━━━━━");
            
            goto_xy(22, 11);
            wprintf(L"비밀번호");
            draw_box(33, 11, 42, 3);
            goto_xy(35, 12);
            if (focused == 0) {
                wprintf(L"%-38ls", new_password);
            } else {
                wprintf(L"%-38ls", new_password);
            }
            
            goto_xy(22, 15);
            wprintf(L"이름");
            draw_box(33, 15, 42, 3);
            goto_xy(35, 16);
            if (focused == 1) {
                wprintf(L"%-28ls", new_name);
            } else {
                wprintf(L"%-28ls", new_name);
            }
            
            draw_box(33, 19, 17, 3);
            goto_xy(38, 20);
            wprintf(L"저 장");
            
            draw_box(53, 19, 17, 3);
            goto_xy(58, 20);
            wprintf(L"취 소");
            
            goto_xy(22, 23);
            wprintf(L"Tab: 다음 | Enter: 저장 | ESC: 취소");
            
            need_redraw = 0;
        }
        
        // 커서 표시
        if (focused == 0) {
            goto_xy(35 + (int)wcslen(new_password), 12);
            set_cursor_visibility(1);
        } else if (focused == 1) {
            goto_xy(35 + (int)wcslen(new_name), 16);
            set_cursor_visibility(1);
        } else {
            set_cursor_visibility(0);
        }
        
        UiInputEvent ev;
        Ui_WaitInput(&ev);
        
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x, my = ev.pos.y;
            
            if (Ui_PointInRect(&rect_password, mx, my)) {
                focused = 0;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_name, mx, my)) {
                focused = 1;
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_save, mx, my)) {
                wcsncpy_s(user->password, 32, new_password, _TRUNCATE);
                wcsncpy_s(user->name, 32, new_name, _TRUNCATE);
                FileIO_UpdateUser(user);
                set_cursor_visibility(0);
                return 1;
            } else if (Ui_PointInRect(&rect_cancel, mx, my)) {
                set_cursor_visibility(0);
                return 0;
            }
        } else if (ev.type == UI_INPUT_KEY) {
            wchar_t ch = ev.key;
            
            if (ch == 27) { // ESC
                set_cursor_visibility(0);
                return 0;
            }
            
            if (ch == L'\t') {
                focused = (focused + 1) % 2;
                need_redraw = 1;
                continue;
            }
            
            if (ch == L'\r' || ch == L'\n') { // Enter
                wcsncpy_s(user->password, 32, new_password, _TRUNCATE);
                wcsncpy_s(user->name, 32, new_name, _TRUNCATE);
                FileIO_UpdateUser(user);
                set_cursor_visibility(0);
                return 1;
            }
            
            wchar_t* buf = (focused == 0) ? new_password : new_name;
            int max_len = 31;
            
            if (ch == L'\b') {
                size_t len = wcslen(buf);
                if (len > 0) buf[len - 1] = L'\0';
                need_redraw = 1;
            } else if (iswprint(ch) || ch == L' ') {
                size_t len = wcslen(buf);
                if (len < max_len) {
                    buf[len] = ch;
                    buf[len + 1] = L'\0';
                    need_redraw = 1;
                }
            }
        }
    }
}

int UserMenu_ConfirmDelete(const wchar_t* user_id) {
    Ui_ClearScreen();
    draw_box(30, 10, 60, 12);
    
    goto_xy(32, 12);
    wprintf(L"??  경고: 회원 탈퇴");
    
    goto_xy(32, 14);
    wprintf(L"모든 데이터가 삭제됩니다.");
    
    goto_xy(32, 15);
    wprintf(L"정말 탈퇴하시겠습니까?");
    
    draw_box(40, 17, 15, 3);
    goto_xy(45, 18);
    wprintf(L"탈퇴");
    
    draw_box(60, 17, 15, 3);
    goto_xy(65, 18);
    wprintf(L"취소");
    
    UiRect rect_delete = {40, 17, 15, 3};
    UiRect rect_cancel = {60, 17, 15, 3};
    
    while (1) {
        UiInputEvent ev;
        Ui_WaitInput(&ev);
        
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            if (Ui_PointInRect(&rect_delete, ev.pos.x, ev.pos.y)) {
                FileIO_DeleteUser(user_id);
                return 1;
            } else if (Ui_PointInRect(&rect_cancel, ev.pos.x, ev.pos.y)) {
                return 0;
            }
        } else if (ev.type == UI_INPUT_KEY && ev.key == 27) {
            return 0;
        }
    }
}
