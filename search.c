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
    
    UiRect rect_input = {25, 8, 60, 1};
    UiRect rect_search = {88, 7, 12, 3};
    
    while (1) {
        if (need_redraw) {
            Ui_ClearScreen();
            draw_box(10, 4, 100, 24);
            goto_xy(1,1);
            wprintf(L"<- Back");
            
            goto_xy(12, 5);
            wprintf(L"━━━━━━━━━━━━━━━━ 일정 검색 ━━━━━━━━━━━━━━━━");
            
            // 검색창
            goto_xy(12, 7);
            wprintf(L"검색어:");
            draw_box(23, 7, 63, 3);
            goto_xy(25, 8);
            if (search_query[0]) {
                wprintf(L"%-58ls", search_query);
            } else {
                SetColor(COLOR_WHITE - 8, COLOR_BLACK);
                wprintf(L"제목 또는 메모로 검색...");
                ResetColor();
            }
            
            draw_box(86, 7, 14, 3);
            goto_xy(89, 8);
            wprintf(L"검 색");
            
            // 검색 결과
            if (searched) {
                goto_xy(12, 11);
                wprintf(L"검색 결과: %d건", result_count);
                
                goto_xy(12, 12);
                for (int i = 0; i < 85; i++) wprintf(L"─");
                
                int y = 14;
                for (int i = 0; i < result_count && i < 10; i++) {
                    // 캘린더 색상 표시
                    Calendar calendars[32];
                    int cal_count = CalMgr_GetAllCalendars(g_current_user.user_id, calendars, 32);
                    int color = COLOR_WHITE;
                    for (int j = 0; j < cal_count; j++) {
                        if (calendars[j].calendar_id == results[i].calendar_id) {
                            color = calendars[j].color;
                            break;
                        }
                    }
                    
                    goto_xy(12, y);
                    SetColor(color, COLOR_BLACK);
                    wprintf(L"■");
                    ResetColor();
                    
                    wprintf(L" %04d-%02d-%02d ",
                            results[i].start_time.tm_year + 1900,
                            results[i].start_time.tm_mon + 1,
                            results[i].start_time.tm_mday);
                    
                    if (!results[i].is_all_day) {
                        wprintf(L"%02d:%02d ",
                                results[i].start_time.tm_hour,
                                results[i].start_time.tm_min);
                    }
                    
                    wprintf(L"%.40ls", results[i].title);
                    
                    y += 2;
                }
                
                if (result_count > 10) {
                    goto_xy(12, y);
                    wprintf(L"외 %d건...", result_count - 10);
                }
            }
            
            goto_xy(12, 26);
            wprintf(L"ESC: 닫기");
            
            need_redraw = 0;
        }
        
        // 커서 표시
        if (search_query[0]) {
            goto_xy(25 + (int)wcslen(search_query), 8);
            set_cursor_visibility(1);
        } else {
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
            
            if (Ui_PointInRect(&rect_input, mx, my)) {
                // 입력 필드 클릭
                need_redraw = 1;
            } else if (Ui_PointInRect(&rect_search, mx, my)) {
                // 검색 실행
                if (search_query[0] == L'\0') {
                    goto_xy(12, 26);
                    wprintf(L"! 검색어를 입력하세요!                    ");
                    Sleep(1500);
                    need_redraw = 1;
                    continue;
                }
                
                // Load all schedules and perform search. Allocate dynamically to reduce stack usage
                Schedule* all_schedules = (Schedule*)malloc(sizeof(Schedule) * 1000);
                int all_count = FileIO_LoadSchedules(all_schedules, 1000);
                
                result_count = 0;
                for (int i = 0; i < all_count && result_count < 100; i++) {
                    if (all_schedules[i].is_deleted) continue;
                    
                    // Search in title or memo
                    if (wcsstr(all_schedules[i].title, search_query) != NULL ||
                        wcsstr(all_schedules[i].memo, search_query) != NULL) {
                        results[result_count++] = all_schedules[i];
                    }
                }
                
                free(all_schedules);
                searched = 1;
                need_redraw = 1;
            }
        } else if (ev.type == UI_INPUT_KEY) {
            wchar_t ch = ev.key;
            
            if (ch == 27) { // ESC
                set_cursor_visibility(0);
                return;
            }
            
            if (ch == L'\r' || ch == L'\n') { // Enter
                // 검색 실행
                if (search_query[0] == L'\0') {
                    goto_xy(12, 26);
                    wprintf(L"! 검색어를 입력하세요!                    ");
                    Sleep(1500);
                    need_redraw = 1;
                    continue;
                }
                
                // Load all schedules and perform search. Allocate dynamically to reduce stack usage
                Schedule* all_schedules = (Schedule*)malloc(sizeof(Schedule) * 1000);
                int all_count = FileIO_LoadSchedules(all_schedules, 1000);
                
                result_count = 0;
                for (int i = 0; i < all_count && result_count < 100; i++) {
                    if (all_schedules[i].is_deleted) continue;
                    
                    // Search in title or memo
                    if (wcsstr(all_schedules[i].title, search_query) != NULL ||
                        wcsstr(all_schedules[i].memo, search_query) != NULL) {
                        results[result_count++] = all_schedules[i];
                    }
                }
                
                free(all_schedules);
                searched = 1;
                need_redraw = 1;
            }
            
            if (ch == L'\b') {
                size_t len = wcslen(search_query);
                if (len > 0) {
                    search_query[len - 1] = L'\0';
                    searched = 0;
                    need_redraw = 1;
                }
            } else if (iswprint(ch) || ch == L' ') {
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
