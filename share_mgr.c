#include "common.h"
#include "share_mgr.h"
#include "ui_core.h"
#include "color_util.h"
#include "file_io.h"
#include "calendar_mgr.h"

extern User g_current_user;

int ShareMgr_GetSharedByMe(const wchar_t* user_id, Share* buf, int max_count) {
    Share all_shares[100];
    int total = FileIO_LoadShares(all_shares, 100);
    
    int result_count = 0;
    for (int i = 0; i < total && result_count < max_count; i++) {
        if (wcscmp(all_shares[i].owner_id, user_id) == 0 && !all_shares[i].is_deleted) {
            buf[result_count++] = all_shares[i];
        }
    }
    return result_count;
}

int ShareMgr_GetSharedToMe(const wchar_t* user_id, Share* buf, int max_count) {
    Share all_shares[100];
    int total = FileIO_LoadShares(all_shares, 100);
    
    int result_count = 0;
    for (int i = 0; i < total && result_count < max_count; i++) {
        if (wcscmp(all_shares[i].shared_with, user_id) == 0 && !all_shares[i].is_deleted) {
            buf[result_count++] = all_shares[i];
        }
    }
    return result_count;
}

int ShareMgr_GetSharedCalendars(const wchar_t* user_id, Calendar* buf, int max_count) {
    Share shares[100];
    int share_count = ShareMgr_GetSharedToMe(user_id, shares, 100);
    
    Calendar all_calendars[100];
    int all_cal_count = FileIO_LoadCalendars(all_calendars, 100);
    
    int result_count = 0;
    for (int i = 0; i < share_count && result_count < max_count; i++) {
        for (int j = 0; j < all_cal_count; j++) {
            if (all_calendars[j].calendar_id == shares[i].calendar_id && !all_calendars[j].is_deleted) {
                buf[result_count++] = all_calendars[j];
                break;
            }
        }
    }
    return result_count;
}

void ShareMgr_Show(void) {
    int tab = 0; // 0: 내가 공유한, 1: 나에게 공유된, 2: 새 공유, 3: 공유 권한 수정
    int need_redraw = 1;

    // 새 공유 만들기 탭
    wchar_t new_user_id[32] = L"";
    int selected_calendar = 0;
    int permission = 0; // 0: 읽기, 1: 편집

    // 공유 권한 수정 탭
    int selected_share_index = 0;       // 리스트에서 선택된 공유 인덱스
    int selected_share_permission = -1; // 0/1이면 토글 값, -1이면 리스트에서 다시 읽기

    UiRect rect_tab1 = (UiRect){ 12, 3, 25, 1 };
    UiRect rect_tab2 = (UiRect){ 38, 3, 25, 1 };
    UiRect rect_tab3 = (UiRect){ 64, 3, 25, 1 };
    UiRect rect_tab4 = (UiRect){ 90, 3, 25, 1 };

    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            draw_box(0, 0, 119, 29);
            goto_xy(1, 1);
            wprintf(L"<- Back");

            // 탭 버튼
            goto_xy(12, 3);
            if (tab == 0) {
                SetColor(COLOR_WHITE, COLOR_BLUE);
                wprintf(L"  내가 공유한 캘린더  ");
                ResetColor();
            }
            else {
                wprintf(L"  내가 공유한 캘린더  ");
            }

            goto_xy(38, 3);
            if (tab == 1) {
                SetColor(COLOR_WHITE, COLOR_BLUE);
                wprintf(L"  나에게 공유된 캘린더  ");
                ResetColor();
            }
            else {
                wprintf(L"  나에게 공유된 캘린더  ");
            }

            goto_xy(64, 3);
            if (tab == 2) {
                SetColor(COLOR_WHITE, COLOR_BLUE);
                wprintf(L"  새 공유 만들기  ");
                ResetColor();
            }
            else {
                wprintf(L"  새 공유 만들기  ");
            }
            goto_xy(90, 3);
            if (tab == 3) {
                SetColor(COLOR_WHITE, COLOR_BLUE);
                wprintf(L"  공유 권한 수정  ");
                ResetColor();
            }
            else {
                wprintf(L"  공유 권한 수정  ");
            }
            goto_xy(7, 5);
            for (int i = 0; i < 106; i++) wprintf(L"─");

            if (tab == 0) {
                // 내가 공유한 캘린더
                Share shares[100];
                int share_count = ShareMgr_GetSharedByMe(g_current_user.user_id, shares, 100);

                Calendar calendars[100];
                int cal_count = FileIO_LoadCalendars(calendars, 100);

                goto_xy(10, 7);
                wprintf(L"캘린더              공유 대상             권한");
                goto_xy(10, 8);
                for (int i = 0; i < 50; i++) wprintf(L"─");

                int y = 10;
                for (int i = 0; i < share_count && i < 15; i++) {
                    // 캘린더 이름 찾기
                    wchar_t cal_name[32] = L"(알 수 없음)";
                    int color = COLOR_WHITE;
                    for (int j = 0; j < cal_count; j++) {
                        if (calendars[j].calendar_id == shares[i].calendar_id) {
                            wcsncpy_s(cal_name, 32, calendars[j].name, _TRUNCATE);
                            color = calendars[j].color;
                            break;
                        }
                    }

                    goto_xy(10, y);
                    SetColor(color, COLOR_BLACK);
                    wprintf(L"■");
                    ResetColor();
                    wprintf(L" %-16ls", cal_name);

                    goto_xy(32, y);
                    wprintf(L"%-16ls", shares[i].shared_with);

                    goto_xy(52, y);
                    wprintf(L"%ls", shares[i].permission == 0 ? L"읽기" : L"편집");

                    y += 2;
                }

                if (share_count == 0) {
                    goto_xy(10, 10);
                    wprintf(L"공유한 캘린더가 없습니다.");
                }

            }
            else if (tab == 1) {
                // 나에게 공유된 캘린더
                Share shares[100];
                int share_count = ShareMgr_GetSharedToMe(g_current_user.user_id, shares, 100);

                Calendar calendars[100];
                int cal_count = FileIO_LoadCalendars(calendars, 100);

                goto_xy(10, 7);
                wprintf(L"캘린더               소유자               권한");
                goto_xy(10, 8);
                for (int i = 0; i < 50; i++) wprintf(L"─");

                int y = 10;
                for (int i = 0; i < share_count && i < 15; i++) {
                    // 캘린더 이름 찾기
                    wchar_t cal_name[32] = L"(알 수 없음)";
                    int color = COLOR_WHITE;
                    for (int j = 0; j < cal_count; j++) {
                        if (calendars[j].calendar_id == shares[i].calendar_id) {
                            wcsncpy_s(cal_name, 32, calendars[j].name, _TRUNCATE);
                            color = calendars[j].color;
                            break;
                        }
                    }

                    goto_xy(10, y);
                    SetColor(color, COLOR_BLACK);
                    wprintf(L"■");
                    ResetColor();
                    wprintf(L" %-16ls", cal_name);

                    goto_xy(32, y);
                    wprintf(L"%-16ls", shares[i].owner_id);

                    goto_xy(52, y);
                    wprintf(L"%ls", shares[i].permission == 0 ? L"읽기" : L"편집");

                    y += 2;
                }

                if (share_count == 0) {
                    goto_xy(10, 10);
                    wprintf(L"공유받은 캘린더가 없습니다.");
                }

            }
            else if (tab == 2) {
                // 새 공유 만들기
                Calendar my_calendars[32];
                int cal_count = CalMgr_GetAllCalendars(g_current_user.user_id, my_calendars, 32);

                goto_xy(10, 7);
                wprintf(L"공유할 캘린더 선택:");

                if (cal_count > 0) {
                    draw_box(35, 6, 40, 3);
                    goto_xy(37, 7);
                    SetColor(my_calendars[selected_calendar].color, COLOR_BLACK);
                    wprintf(L"■");
                    ResetColor();
                    wprintf(L" %ls", my_calendars[selected_calendar].name);

                    goto_xy(78, 7);
                    wprintf(L"◀ ▶");
                }
                else {
                    goto_xy(35, 7);
                    wprintf(L"(공유 가능한 캘린더 없음)");
                }

                goto_xy(10, 11);
                wprintf(L"공유 대상 사용자 ID:");
                draw_box(35, 10, 40, 3);
                goto_xy(37, 11);
                if (new_user_id[0]) {
                    wprintf(L"%-36ls", new_user_id);
                }
                else {
                    SetColor(COLOR_WHITE - 8, COLOR_BLACK);
                    wprintf(L"사용자 ID 입력");
                    ResetColor();
                }

                goto_xy(10, 15);
                wprintf(permission == 0 ? L"[v] 읽기 전용" : L"[ ] 읽기 전용");

                goto_xy(10, 17);
                wprintf(permission == 1 ? L"[v] 편집 가능" : L"[ ] 편집 가능");

                draw_box(40, 20, 15, 3);
                goto_xy(44, 21);
                wprintf(L"공유하기");
            }
            else if (tab == 3) {
                // 공유 권한 수정 / 해제
                Share shares[100];
                int share_count = ShareMgr_GetSharedByMe(g_current_user.user_id, shares, 100);

                Calendar calendars[100];
                int cal_count = FileIO_LoadCalendars(calendars, 100);

                goto_xy(10, 7);
                wprintf(L"공유 권한 수정 및 해제");

                if (share_count == 0) {
                    goto_xy(10, 10);
                    wprintf(L"공유한 캘린더가 없습니다.");
                }
                else {
                    // 선택 인덱스 보정
                    if (selected_share_index < 0) selected_share_index = 0;
                    if (selected_share_index >= share_count) selected_share_index = share_count - 1;

                    goto_xy(10, 9);
                    wprintf(L"선택  캘린더           공유 대상           권한");
                    goto_xy(10, 10);
                    for (int i = 0; i < 50; i++) wprintf(L"─");

                    int y = 12;
                    for (int i = 0; i < share_count && i < 10; i++) {
                        // 캘린더 이름/색 찾기
                        wchar_t cal_name[32] = L"(알 수 없음)";
                        int color = COLOR_WHITE;
                        for (int j = 0; j < cal_count; j++) {
                            if (calendars[j].calendar_id == shares[i].calendar_id) {
                                wcsncpy_s(cal_name, 32, calendars[j].name, _TRUNCATE);
                                color = calendars[j].color;
                                break;
                            }
                        }

                        // 선택 표시
                        goto_xy(10, y);
                        if (i == selected_share_index) {
                            wprintf(L">");
                        }
                        else {
                            wprintf(L" ");
                        }

                        // 캘린더 이름
                        goto_xy(12, y);
                        SetColor(color, COLOR_BLACK);
                        wprintf(L"■");
                        ResetColor();
                        wprintf(L" %-16ls", cal_name);

                        // 공유 대상
                        goto_xy(34, y);
                        wprintf(L"%-16ls", shares[i].shared_with);

                        // 권한
                        goto_xy(54, y);
                        wprintf(L"%ls", shares[i].permission == 0 ? L"읽기" : L"편집");

                        y += 2;
                    }

                    // 현재 권한 값 결정
                    int current_perm;
                    if (selected_share_permission == 0 || selected_share_permission == 1) {
                        current_perm = selected_share_permission;
                    }
                    else {
                        current_perm = shares[selected_share_index].permission;
                        selected_share_permission = current_perm;
                    }

                    // 권한 토글 UI
                    goto_xy(10, 22);
                    wprintf(current_perm == 0 ? L"[v] 읽기 전용" : L"[ ] 읽기 전용");

                    goto_xy(30, 22);
                    wprintf(current_perm == 1 ? L"[v] 편집 가능" : L"[ ] 편집 가능");

                    // 버튼: 권한 저장 / 공유 해제
                    draw_box(40, 24, 15, 3);
                    goto_xy(44, 25);
                    wprintf(L"권한 저장");

                    draw_box(60, 24, 15, 3);
                    goto_xy(64, 25);
                    wprintf(L"공유 해제");
                }


                need_redraw = 0;
            }

            // 커서 표시
            if (tab == 2 && new_user_id[0]) {
                goto_xy(37 + (int)wcslen(new_user_id), 11);
                set_cursor_visibility(1);
            }
            else {
                set_cursor_visibility(0);
            }

            UiInputEvent ev;
            Ui_WaitInput(&ev);

            if (ev.type == UI_INPUT_MOUSE_LEFT) {
                int mx = ev.pos.x, my = ev.pos.y;
                // back button click
                if (my == 1 && mx >= 1 && mx <= 7) {
                    set_cursor_visibility(0);
                    return;
                }

                // 탭 전환
                if (Ui_PointInRect(&rect_tab1, mx, my)) {
                    tab = 0;
                    need_redraw = 1;
                }
                else if (Ui_PointInRect(&rect_tab2, mx, my)) {
                    tab = 1;
                    need_redraw = 1;
                }
                else if (Ui_PointInRect(&rect_tab3, mx, my)) {
                    tab = 2;
                    need_redraw = 1;
                }
                else if (Ui_PointInRect(&rect_tab4, mx, my)) {
                    tab = 3;
                    selected_share_index = 0;
                    selected_share_permission = -1; // 처음엔 리스트 값에서 다시 읽기
                    need_redraw = 1;
                }

                // 새 공유 만들기 탭
                if (tab == 2) {
                    Calendar my_calendars[32];
                    int cal_count = CalMgr_GetAllCalendars(g_current_user.user_id, my_calendars, 32);

                    // 캘린더 선택 화살표
                    if (my >= 6 && my <= 8 && mx >= 78 && mx <= 79) { // ◀
                        if (cal_count > 0) {
                            selected_calendar = (selected_calendar - 1 + cal_count) % cal_count;
                            need_redraw = 1;
                        }
                    }
                    else if (my >= 6 && my <= 8 && mx >= 81 && mx <= 82) { // ▶
                        if (cal_count > 0) {
                            selected_calendar = (selected_calendar + 1) % cal_count;
                            need_redraw = 1;
                        }
                    }

                    // 사용자 ID 입력
                    else if (my >= 10 && my <= 12 && mx >= 37 && mx <= 74) {
                        need_redraw = 1;
                    }

                    // 권한 선택
                    else if (my == 15 && mx >= 10 && mx <= 23) {
                        permission = 0;
                        need_redraw = 1;
                    }
                    else if (my == 17 && mx >= 10 && mx <= 23) {
                        permission = 1;
                        need_redraw = 1;
                    }

                    // 공유하기 버튼
                    else if (my >= 20 && my <= 22 && mx >= 40 && mx <= 54) {
                        if (cal_count == 0) {
                            goto_xy(10, 26);
                            wprintf(L"! 공유할 캘린더가 없습니다!              ");
                            Sleep(1500);
                            need_redraw = 1;
                        }
                        else if (new_user_id[0] == L'\0') {
                            goto_xy(10, 26);
                            wprintf(L"! 사용자 ID를 입력하세요!               ");
                            Sleep(1500);
                            need_redraw = 1;
                        }
                        else {
                            // 공유 생성
                            Share new_share = { 0 };
                            new_share.calendar_id = my_calendars[selected_calendar].calendar_id;
                            wcsncpy_s(new_share.owner_id, 32, g_current_user.user_id, _TRUNCATE);
                            wcsncpy_s(new_share.shared_with, 32, new_user_id, _TRUNCATE);
                            new_share.permission = permission;
                            new_share.is_deleted = 0;

                            FileIO_AddShare(&new_share);

                            goto_xy(10, 26);
                            wprintf(L"! 공유가 완료되었습니다!                 ");
                            Sleep(1500);

                            new_user_id[0] = L'\0';
                            tab = 0;
                            need_redraw = 1;
                        }
                    }
                }
                else if (tab == 3) {
                    Share shares[100];
                    int share_count = ShareMgr_GetSharedByMe(g_current_user.user_id, shares, 100);

                    if (share_count > 0) {
                        // 1) 리스트에서 항목 선택 (행 클릭)
                        int y = 12;
                        for (int i = 0; i < share_count && i < 10; i++) {
                            if (my == y && mx >= 10 && mx <= 70) {
                                selected_share_index = i;
                                selected_share_permission = -1; // 실제 값에서 다시 읽기
                                need_redraw = 1;
                                break;
                            }
                            y += 2;
                        }

                        // 2) 권한 토글 클릭
                        if (my == 22 && mx >= 10 && mx <= 24) {
                            selected_share_permission = 0; // 읽기 전용
                            need_redraw = 1;
                        }
                        else if (my == 22 && mx >= 30 && mx <= 44) {
                            selected_share_permission = 1; // 편집 가능
                            need_redraw = 1;
                        }

                        if (my >= 24 && my <= 26 && mx >= 40 && mx <= 54) {
                            if (selected_share_index >= 0 && selected_share_index < share_count) {
                                int perm_to_save;
                                if (selected_share_permission == 0 || selected_share_permission == 1) {
                                    perm_to_save = selected_share_permission;
                                }
                                else {
                                    perm_to_save = shares[selected_share_index].permission;
                                }

                                FileIO_UpdateSharePermission(shares[selected_share_index].share_id, perm_to_save);

                                // ★ 여기서 화면을 나가지 않고, 메세지만 보여주고 다시 그려준다
                                goto_xy(10, 27);
                                wprintf(L"! 권한이 저장되었습니다!                 ");
                                Sleep(1500);

                                selected_share_permission = -1;
                                need_redraw = 1;

                            }
                        }
                        // 4) 공유 해제 버튼 (60~74, 24~26)
                        else if (my >= 24 && my <= 26 && mx >= 60 && mx <= 74) {
                            if (selected_share_index >= 0 && selected_share_index < share_count) {
                                int deleted_id = shares[selected_share_index].share_id;
                                FileIO_DeleteShare(deleted_id);

                                if (selected_share_index > 0) {
                                    selected_share_index--;
                                }
                                selected_share_permission = -1;
                                need_redraw = 1;
                            }
                        }
                    }
                }

            }
            else if (ev.type == UI_INPUT_KEY) {
                wchar_t ch = ev.key;

                if (ch == 27) { // ESC
                    set_cursor_visibility(0);
                    return;
                }

                // 새 공유 만들기 탭에서 입력
                if (tab == 2) {
                    if (ch == L'\b') {
                        size_t len = wcslen(new_user_id);
                        if (len > 0) {
                            new_user_id[len - 1] = L'\0';
                            need_redraw = 1;
                        }
                    }
                    else if (iswprint(ch) && ch != L' ') {
                        size_t len = wcslen(new_user_id);
                        if (len < 31) {
                            new_user_id[len] = ch;
                            new_user_id[len + 1] = L'\0';
                            need_redraw = 1;
                        }
                    }
                }
            }
        }
    }
}
