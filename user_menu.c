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

    // 화면/필드 폭 기준으로 가운데 정렬 좌표 계산
    const int SCREEN_W = 120;
    const int FIELD_BOX_W = 42;
    const int FIELD_LABEL_OFF = 11;
    const int BUTTON_BOX_W = 17;
    const int BUTTON_GAP = 6;

    int field_box_x = (SCREEN_W - FIELD_BOX_W) / 2;           // 입력 박스 시작 x
    int pw_label_x = field_box_x - FIELD_LABEL_OFF;          // "비밀번호" 레이블 x
    int name_label_x = pw_label_x;                             // "이름" 레이블 x
    int pw_box_x = field_box_x;
    int name_box_x = field_box_x;
    int pw_text_x = field_box_x + 2;                        // 비밀번호 입력 시작 x
    int name_text_x = field_box_x + 2;                        // 이름 입력 시작 x

    int btn_group_w = BUTTON_BOX_W * 2 + BUTTON_GAP;          // 두 버튼 + 간격
    int btn_group_x = (SCREEN_W - btn_group_w) / 2;           // 버튼 그룹 시작 x
    int btn_box_y = 19;                                     // 버튼 박스 y (원래와 비슷하게 유지)

    int btn_save_box_x = btn_group_x;
    int btn_cancel_box_x = btn_group_x + BUTTON_BOX_W + BUTTON_GAP;

    UiRect rect_password = { pw_text_x, 12, FIELD_BOX_W - 2, 1 };
    UiRect rect_name = { name_text_x, 16, FIELD_BOX_W - 2, 1 };
    UiRect rect_save = { btn_save_box_x + 2,   btn_box_y + 1, 15, 3 };
    UiRect rect_cancel = { btn_cancel_box_x + 2, btn_box_y + 1, 15, 3 };

    int need_redraw = 1;

    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            draw_box(0, 0, 119, 29);

            // 제목은 대략 가운데에 오도록 x만 계산
            int title_x = (SCREEN_W - 22) / 2; 
            goto_xy(title_x-10, 9);
            wprintf(L"━━━━━━━━━━━━━ 회원정보 수정 ━━━━━━━━━━━━━");

            // 비밀번호 필드
            goto_xy(pw_label_x, 12);
            wprintf(L"비밀번호");
            draw_box(pw_box_x, 11, FIELD_BOX_W, 3);
            goto_xy(pw_text_x, 12);
            wprintf(L"%-38ls", new_password);

            // 이름 필드
            goto_xy(name_label_x, 16);
            wprintf(L"이름");
            draw_box(name_box_x, 15, FIELD_BOX_W, 3);
            goto_xy(name_text_x, 16);
            wprintf(L"%-28ls", new_name);

            // 저장 버튼
            draw_box(btn_save_box_x, btn_box_y, BUTTON_BOX_W, 3);
            goto_xy(btn_save_box_x + 5, btn_box_y + 1);
            wprintf(L"저 장");

            // 취소 버튼
            draw_box(btn_cancel_box_x, btn_box_y, BUTTON_BOX_W, 3);
            goto_xy(btn_cancel_box_x + 5, btn_box_y + 1);
            wprintf(L"취 소");

            need_redraw = 0;
        }

        // 커서 표시 (포커스 필드 끝 위치)
        if (focused == 0) {
            goto_xy(pw_text_x + (int)wcslen(new_password), 12);
            set_cursor_visibility(1);
        }
        else if (focused == 1) {
            goto_xy(name_text_x + (int)wcslen(new_name), 16);
            set_cursor_visibility(1);
        }
        else {
            set_cursor_visibility(0);
        }

        UiInputEvent ev;
        Ui_WaitInput(&ev);

        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x, my = ev.pos.y;

            if (Ui_PointInRect(&rect_password, mx, my)) {
                focused = 0;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_name, mx, my)) {
                focused = 1;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_save, mx, my)) {
                wcsncpy_s(user->password, 32, new_password, _TRUNCATE);
                wcsncpy_s(user->name, 32, new_name, _TRUNCATE);
                FileIO_UpdateUser(user);
                set_cursor_visibility(0);
                return 1;
            }
            else if (Ui_PointInRect(&rect_cancel, mx, my)) {
                set_cursor_visibility(0);
                return 0;
            }
        }
        else if (ev.type == UI_INPUT_KEY) {
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
            }
            else if (iswprint(ch) || ch == L' ') {
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

    // 화면/박스 크기를 기준으로 가운데 정렬 좌표 계산
    const int SCREEN_W = 120;
    const int SCREEN_H = 30;
    const int BOX_W = 60;
    const int BOX_H = 12;

    int box_x = (SCREEN_W - BOX_W) / 2;  // 30
    int box_y = (SCREEN_H - BOX_H) / 2;  // 9

    draw_box(box_x, box_y, BOX_W, BOX_H);

    goto_xy(box_x + 2, box_y + 2);
    wprintf(L"??  경고: 회원 탈퇴");

    goto_xy(box_x + 2, box_y + 4);
    wprintf(L"모든 데이터가 삭제됩니다.");

    goto_xy(box_x + 2, box_y + 5);
    wprintf(L"정말 탈퇴하시겠습니까?");

    // 버튼 박스는 팝업 박스 안에서 좌우로 배치
    int btn_box_y = box_y + 7;
    int btn_delete_x = box_x + 8;
    int btn_cancel_x = box_x + BOX_W - 8 - 15; // 오른쪽에서 8칸 + 버튼폭 15

    draw_box(btn_delete_x, btn_box_y, 15, 3);
    goto_xy(btn_delete_x + 4, btn_box_y + 1);
    wprintf(L"탈퇴");

    draw_box(btn_cancel_x, btn_box_y, 15, 3);
    goto_xy(btn_cancel_x + 4, btn_box_y + 1);
    wprintf(L"취소");

    UiRect rect_delete = { btn_delete_x, btn_box_y, 15, 3 };
    UiRect rect_cancel = { btn_cancel_x, btn_box_y, 15, 3 };

    while (1) {
        UiInputEvent ev;
        Ui_WaitInput(&ev);

        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            if (Ui_PointInRect(&rect_delete, ev.pos.x, ev.pos.y)) {
                FileIO_DeleteUser(user_id);
                return 1;
            }
            else if (Ui_PointInRect(&rect_cancel, ev.pos.x, ev.pos.y)) {
                return 0;
            }
        }
        else if (ev.type == UI_INPUT_KEY && ev.key == 27) {
            return 0;
        }
    }
}