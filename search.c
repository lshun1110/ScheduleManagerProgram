// search.c
#include "common.h"
#include "search.h"
#include "ui_core.h"
#include "color_util.h"
#include "file_io.h"
#include "calendar_mgr.h"

#include <stdlib.h> // for malloc/free

extern User g_current_user;

void Search_ShowDialog(void) {
    wchar_t search_query[64] = L"";
    int need_redraw = 1;

    Schedule results[100];
    int result_count = 0;
    int searched = 0;
    int placeholder_cleared = 0;  // 검색창 안내 문구(플레이스홀더) 상태

    UiRect rect_input = (UiRect){ 25, 8, 60, 1 };
    UiRect rect_search = (UiRect){ 88, 7, 12, 3 };

    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            draw_box(0, 0, 119, 29);
            goto_xy(1, 1);
            wprintf(L"<- Back");

            goto_xy(12, 5);
            wprintf(L"━━━━━━━━━━━━━━━━ 일정 검색 ━━━━━━━━━━━━━━━━");

            // ───────────────── 검색창 ─────────────────
            goto_xy(12, 8);
            wprintf(L"검색어:");
            draw_box(23, 7, 63, 3);

            // 먼저 검색창 안을 공백으로 싹 밀어버리고
            goto_xy(25, 8);
            wprintf(L"                                                          "); // 대충 58칸 정도 공백
            goto_xy(25, 8);

            if (search_query[0]) {
                // 이미 입력된 검색어가 있으면 그대로 출력
                wprintf(L"%-58ls", search_query);
            }
            else {
                // 아직 입력 전: 플레이스홀더 표시 (클릭 전까지만)
                if (!placeholder_cleared) {
                    SetColor(COLOR_WHITE - 8, COLOR_BLACK);
                    wprintf(L"제목 또는 메모로 검색...");
                    ResetColor();
                }
                // placeholder_cleared == 1 이면서 search_query 비어있으면 그냥 빈 칸 유지
            }

            // 검색 버튼
            draw_box(86, 7, 14, 3);
            goto_xy(89, 8);
            wprintf(L"검 색");

            // ───────────────── 검색 결과 ─────────────────
            if (searched) {
                goto_xy(12, 11);
                wprintf(L"검색 결과: %d건", result_count);

                int max_show = (result_count < 10) ? result_count : 10;
                for (int i = 0; i < max_show; i++) {
                    Schedule* s = &results[i];
                    int row = 13 + i * 2;

                    goto_xy(12, row);
                    wprintf(L"[%d] ", s->schedule_id);

                    // 날짜/시간
                    goto_xy(18, row);
                    wprintf(L"%04d-%02d-%02d %02d:%02d",
                        s->start_time.tm_year + 1900,
                        s->start_time.tm_mon + 1,
                        s->start_time.tm_mday,
                        s->start_time.tm_hour,
                        s->start_time.tm_min);

                    // 제목
                    goto_xy(40, row);
                    wprintf(L"%-20.20ls", s->title);

                    // 캘린더 ID
                    goto_xy(62, row);
                    wprintf(L"(Cal:%d)", s->calendar_id);
                }

                if (result_count > 10) {
                    goto_xy(12, 13 + 10 * 2);
                    wprintf(L"외 %d건...", result_count - 10);
                }
            }
            else {
                goto_xy(12, 11);
                wprintf(L"검색 결과가 없습니다. 검색어를 입력하고 [검 색] 버튼을 누르세요.");
            }

            need_redraw = 0;
        }

        // 한글 IME 때문에, 입력 받기 전에 항상 커서를 검색창으로 고정
        goto_xy(25 + (int)wcslen(search_query), 8);
        set_cursor_visibility(1);

        // ───────────────── 입력 처리 ─────────────────
        UiInputEvent ev;
        Ui_WaitInput(&ev);

        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x;
            int my = ev.pos.y;

            // Back
            if (my == 1 && mx >= 1 && mx <= 8) {
                set_cursor_visibility(0);
                return;
            }

            if (Ui_PointInRect(&rect_input, mx, my)) {
                // 입력 필드 클릭 → 플레이스홀더 제거
                placeholder_cleared = 1;
                need_redraw = 1;
            }
            else if (Ui_PointInRect(&rect_search, mx, my)) {
                // 검색 실행 (마우스로 버튼 클릭)
                if (search_query[0] == L'\0') {
                    goto_xy(12, 26);
                    wprintf(L"! 검색어를 입력하세요!                    ");
                    Sleep(1500);
                    need_redraw = 1;
                    continue;
                }

                // 모든 일정 로드
                Schedule* all_schedules = (Schedule*)malloc(sizeof(Schedule) * 1000);
                int all_count = FileIO_LoadSchedules(all_schedules, 1000);

                // 현재 사용자 기준으로 접근 가능한 캘린더 목록 로드
                Calendar user_cals[32];
                int user_cal_count = CalMgr_GetAllCalendars(g_current_user.user_id, user_cals, 32);

                result_count = 0;
                for (int i = 0; i < all_count && result_count < 100; i++) {
                    Schedule* s = &all_schedules[i];
                    if (s->is_deleted) continue;

                    int owned = 0;
                    for (int j = 0; j < user_cal_count; j++) {
                        if (user_cals[j].calendar_id == s->calendar_id) {
                            owned = 1;
                            break;
                        }
                    }
                    if (!owned) continue;

                    if (wcsstr(s->title, search_query) != NULL ||
                        wcsstr(s->memo, search_query) != NULL) {
                        results[result_count++] = *s;
                    }
                }

                free(all_schedules);
                searched = 1;
                need_redraw = 1;
            }
        }
        else if (ev.type == UI_INPUT_KEY) {
            wchar_t ch = ev.key;

            if (ch == 27) { // ESC
                set_cursor_visibility(0);
                return;
            }

            if (ch == L'\r' || ch == L'\n') { // Enter로 검색 실행
                if (search_query[0] == L'\0') {
                    goto_xy(12, 26);
                    wprintf(L"! 검색어를 입력하세요!                    ");
                    Sleep(1500);
                    need_redraw = 1;
                    continue;
                }

                Schedule* all_schedules = (Schedule*)malloc(sizeof(Schedule) * 1000);
                int all_count = FileIO_LoadSchedules(all_schedules, 1000);

                Calendar user_cals[32];
                int user_cal_count = CalMgr_GetAllCalendars(g_current_user.user_id, user_cals, 32);

                result_count = 0;
                for (int i = 0; i < all_count && result_count < 100; i++) {
                    Schedule* s = &all_schedules[i];
                    if (s->is_deleted) continue;

                    int owned = 0;
                    for (int j = 0; j < user_cal_count; j++) {
                        if (user_cals[j].calendar_id == s->calendar_id) {
                            owned = 1;
                            break;
                        }
                    }
                    if (!owned) continue;

                    if (wcsstr(s->title, search_query) != NULL ||
                        wcsstr(s->memo, search_query) != NULL) {
                        results[result_count++] = *s;
                    }
                }

                free(all_schedules);
                searched = 1;
                need_redraw = 1;
            }

            if (ch == L'\b') {  // 백스페이스
                size_t len = wcslen(search_query);
                if (len > 0) {
                    search_query[len - 1] = L'\0';
                    searched = 0;
                    need_redraw = 1;
                }
            }
            else if (iswprint(ch) || ch == L' ') {
                size_t len = wcslen(search_query);
                if (len < 63) {
                    search_query[len] = ch;
                    search_query[len + 1] = L'\0';
                    searched = 0;
                    need_redraw = 1;
                }
            }
        }
    }
}