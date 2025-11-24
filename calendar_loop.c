// calendar_loop.c
#include "common.h"
#include "calendar_loop.h"
#include "calendar_ui.h"
#include "ui_core.h"
#include "app_scene.h"
#include "schedule_logic.h"
#include "schedule_dialog.h"
#include "calendar_mgr.h"
#include "file_io.h"
#include "stats.h"
#include "search.h"
#include "user_menu.h"
#include "share_mgr.h"
#include "calendar_dialog.h"

extern User g_current_user;

SceneState Calendar_Loop(void)
{
    time_t now_t = time(NULL);
    struct tm today;
    localtime_s(&today, &now_t);

    struct tm current_month = today;
    current_month.tm_mday = 1;
    mktime(&current_month);

    struct tm selected_date = today;
    
    ViewType view_type = VIEW_MONTHLY;
    
    UiRect rect_prev, rect_next, rect_today, rect_logout;
    UiRect rect_view_toggle, rect_stats, rect_search, rect_user_menu;
    UiRect rect_cells[6][7];
    UiRect rect_week_cells[7];
    UiRect rect_new_schedule;
    UiRect rect_new_calendar, rect_share_mgr;
    
    CalendarCheckbox calendars[32];
    int calendar_count = 0;
    
    int need_redraw = 1;
    
    while (1)
    {
        if (need_redraw) {
            Ui_ClearScreen();
            set_cursor_visibility(0);
            
            // 헤더 그리기
            CalendarUi_DrawHeader(&current_month, &selected_date,
                                 &rect_prev, &rect_next, &rect_today,
                                 &rect_logout, &rect_view_toggle,
                                 &rect_stats, &rect_search, &rect_user_menu,
                                 view_type);
            
            // 왼쪽 패널 그리기 (캘린더 목록)
            CalendarUi_DrawLeftPanel(calendars, &calendar_count,
                                    &rect_new_calendar, &rect_share_mgr);
            
            // 활성화된 캘린더 ID 수집
            int active_calendar_ids[32];
            int active_count = 0;
            for (int i = 0; i < calendar_count; i++) {
                if (calendars[i].is_checked) {
                    active_calendar_ids[active_count++] = calendars[i].calendar_id;
                }
            }
            
            // 달력 그리기
            if (view_type == VIEW_MONTHLY) {
                CalendarUi_DrawMonthGrid(&current_month, &selected_date,
                                        active_calendar_ids, active_count,
                                        rect_cells);
            } else {
                // 주간 뷰: 선택된 날짜가 속한 주의 월요일 계산
                struct tm week_start = selected_date;
                int days_to_monday = (selected_date.tm_wday + 6) % 7;
                week_start.tm_mday -= days_to_monday;
                mktime(&week_start);
                
                CalendarUi_DrawWeekGrid(&week_start, &selected_date,
                                       active_calendar_ids, active_count,
                                       rect_week_cells);
            }
            
            // 오른쪽 패널 그리기 (일정 목록)
            int active_calendar_ids_right[32];
            int active_count_right = 0;
            for (int i = 0; i < calendar_count; i++) {
                if (calendars[i].is_checked) {
                    active_calendar_ids_right[active_count_right++] = calendars[i].calendar_id;
                }
            }
            if (view_type == VIEW_MONTHLY) {
                CalendarUi_DrawRightPanel(&selected_date, active_calendar_ids_right, active_count_right,
                                     &rect_new_schedule);
            }
            
            need_redraw = 0;
        }
        
        UiInputEvent ev;
        if (!Ui_WaitInput(&ev)) continue;
        
        if (ev.type == UI_INPUT_MOUSE_LEFT) {
            int mx = ev.pos.x, my = ev.pos.y;
            
            // 로그아웃
            if (Ui_PointInRect(&rect_logout, mx, my)) {
                return SCENE_LOGIN;
            }
            
            // 사용자 메뉴
            if (Ui_PointInRect(&rect_user_menu, mx, my)) {
                UserMenuAction action = UserMenu_Show(rect_user_menu.x, rect_user_menu.y + 1);
                
                if (action == USER_MENU_PROFILE) {
                    UserMenu_EditProfile(&g_current_user);
                } else if (action == USER_MENU_SHARE) {
                    ShareMgr_Show();
                } else if (action == USER_MENU_DELETE) {
                    if (UserMenu_ConfirmDelete(g_current_user.user_id)) {
                        return SCENE_LOGIN;
                    }
                }
                need_redraw = 1;
            }
            
            // 이전 월
            if (Ui_PointInRect(&rect_prev, mx, my)) {
                if (view_type == VIEW_MONTHLY) {
                    if (--current_month.tm_mon < 0) {
                        current_month.tm_mon = 11;
                        current_month.tm_year--;
                    }
                    current_month.tm_mday = 1;
                    mktime(&current_month);
                } else {
                    // 주간 뷰: 이전 주
                    selected_date.tm_mday -= 7;
                    mktime(&selected_date);
                }
                need_redraw = 1;
            }
            
            // 다음 월
            if (Ui_PointInRect(&rect_next, mx, my)) {
                if (view_type == VIEW_MONTHLY) {
                    if (++current_month.tm_mon > 11) {
                        current_month.tm_mon = 0;
                        current_month.tm_year++;
                    }
                    current_month.tm_mday = 1;
                    mktime(&current_month);
                } else {
                    // 주간 뷰: 다음 주
                    selected_date.tm_mday += 7;
                    mktime(&selected_date);
                }
                need_redraw = 1;
            }
            
            // 오늘
            if (Ui_PointInRect(&rect_today, mx, my)) {
                selected_date = today;
                current_month = today;
                current_month.tm_mday = 1;
                mktime(&current_month);
                need_redraw = 1;
            }
            
            // 뷰 전환 (월간/주간)
            if (Ui_PointInRect(&rect_view_toggle, mx, my)) {
                view_type = (view_type == VIEW_MONTHLY) ? VIEW_WEEKLY : VIEW_MONTHLY;
                need_redraw = 1;
            }
            
            // 통계
            if (Ui_PointInRect(&rect_stats, mx, my)) {
                if (view_type == VIEW_MONTHLY) {
                    Stats_ShowMonthly(&current_month);
                } else {
                    // 주간 통계
                    struct tm week_start = selected_date;
                    int days_to_monday = (selected_date.tm_wday + 6) % 7;
                    week_start.tm_mday -= days_to_monday;
                    mktime(&week_start);
                    Stats_ShowWeekly(&week_start);
                }
                need_redraw = 1;
            }
            
            // 검색
            if (Ui_PointInRect(&rect_search, mx, my)) {
                Search_ShowDialog();
                need_redraw = 1;
            }
            
            // 연도/월 클릭
            if (my == 4 && mx >= 30 && mx <= 46) {
                if (CalendarUi_ShowYearMonthDialog(&current_month)) {
                    need_redraw = 1;
                }
            }
            
            // 캘린더 체크박스 클릭
            for (int i = 0; i < calendar_count; i++) {
                if (Ui_PointInRect(&calendars[i].rect, mx, my)) {
                    calendars[i].is_checked = !calendars[i].is_checked;
                    
                    // 파일에도 반영
                    Calendar cal;
                    Calendar all_cals[100];
                    int all_count = FileIO_LoadCalendars(all_cals, 100);
                    
                    for (int j = 0; j < all_count; j++) {
                        if (all_cals[j].calendar_id == calendars[i].calendar_id) {
                            all_cals[j].is_active = calendars[i].is_checked;
                            FileIO_UpdateCalendar(&all_cals[j]);
                            break;
                        }
                    }
                    
                    need_redraw = 1;
                    break;
                }
            }
            
            // 새 캘린더
            if (Ui_PointInRect(&rect_new_calendar, mx, my)) {
                // 캘린더 생성 다이얼로그 화면 띄우기
                CalendarDialog_Add(g_current_user.user_id);
                // 다이얼로그에서 나왔으면 리스트 다시 그리기
                need_redraw = 1;
            }
            
            // 공유 관리
            if (Ui_PointInRect(&rect_share_mgr, mx, my)) {
                ShareMgr_Show();
                need_redraw = 1;
            }
            
            // 날짜 셀 클릭 (월간 뷰)
            if (view_type == VIEW_MONTHLY) {
                for (int row = 0; row < 6; row++) {
                    for (int col = 0; col < 7; col++) {
                        if (Ui_PointInRect(&rect_cells[row][col], mx, my)) {
                            struct tm first_day = current_month;
                            first_day.tm_mday = 1;
                            mktime(&first_day);
                            
                            int first_wday = (first_day.tm_wday + 6) % 7;
                            int day_index = row * 7 + col;
                            
                            if (day_index >= first_wday) {
                                struct tm new_sel = current_month;
                                new_sel.tm_mday = day_index - first_wday + 1;
                                mktime(&new_sel);
                                
                                if (new_sel.tm_mon == current_month.tm_mon) {
                                    // 같은 날짜를 다시 클릭하면 선택 해제 (원래 색으로)
                                    if (selected_date.tm_year == new_sel.tm_year &&
                                        selected_date.tm_mon == new_sel.tm_mon &&
                                        selected_date.tm_mday == new_sel.tm_mday) {
                                        // 선택 해제는 구현하지 않음 (항상 선택 상태 유지)
                                    } else {
                                        selected_date = new_sel;
                                    }
                                    need_redraw = 1;
                                }
                            }
                            goto end_cell_click;
                        }
                    }
                }
                end_cell_click:;
            }
            
            // 새 일정 등록 버튼
                // handle schedule item click for editing
    if (view_type == VIEW_MONTHLY) {
        int active_ids_edit[32];
        int active_cnt_edit = 0;
        for (int i = 0; i < calendar_count; i++) {
            if (calendars[i].is_checked) {
                active_ids_edit[active_cnt_edit++] = calendars[i].calendar_id;
            }
        }
        Schedule sbuf_edit[50];
        int scount_edit = CalMgr_GetSchedulesForDate(active_ids_edit, active_cnt_edit, &selected_date, sbuf_edit, 50);
        int schedStartX = 82;
        int schedStartY = 10;
        int maxRows = (scount_edit > 12 ? 12 : scount_edit);
        if (mx >= schedStartX && mx < schedStartX + 36 && my >= schedStartY && my < schedStartY + maxRows) {
            int idx = my - schedStartY;
            if (idx >= 0 && idx < scount_edit) {
                Schedule temp = sbuf_edit[idx];
                Calendar all_cals_edit[32];
                int all_count_edit = CalMgr_GetAllCalendars(g_current_user.user_id, all_cals_edit, 32);
                if (ScheduleDialog_Edit(all_cals_edit, all_count_edit, &temp)) {
                }
                need_redraw = 1;
            }
        }
    } else {
        for (int col = 0; col < 7; col++) {
            if (Ui_PointInRect(&rect_week_cells[col], mx, my)) {
                int relativeY = my - rect_week_cells[col].y;
                int rowIndex = relativeY / 2;
                struct tm day = selected_date;
                int days_to_monday = (selected_date.tm_wday + 6) % 7;
                day.tm_mday -= days_to_monday;
                day.tm_mday += col;
                mktime(&day);
                int active_ids_w[32];
                int active_cnt_w = 0;
                for (int i = 0; i < calendar_count; i++) {
                    if (calendars[i].is_checked) {
                        active_ids_w[active_cnt_w++] = calendars[i].calendar_id;
                    }
                }
                Schedule sbuf_w[50];
                int scount_w = CalMgr_GetSchedulesForDate(active_ids_w, active_cnt_w, &day, sbuf_w, 50);
                for (int i = 1; i < scount_w; i++) {
                    Schedule key = sbuf_w[i];
                    int j = i - 1;
                    while (j >= 0) {
                        time_t t_j = mktime(&sbuf_w[j].start_time);
                        time_t t_key = mktime(&key.start_time);
                        if (t_j > t_key) {
                            sbuf_w[j + 1] = sbuf_w[j];
                            j--;
                        } else break;
                    }
                    sbuf_w[j + 1] = key;
                }
                int maxRowsW = (scount_w > 6 ? 6 : scount_w);
                if (rowIndex >= 0 && rowIndex < maxRowsW) {
                    Schedule temp = sbuf_w[rowIndex];
                    Calendar all_cals_edit2[32];
                    int all_count_edit2 = CalMgr_GetAllCalendars(g_current_user.user_id, all_cals_edit2, 32);
                    if (ScheduleDialog_Edit(all_cals_edit2, all_count_edit2, &temp)) {
                    }
                    need_redraw = 1;
                }
                break;
            }
        }
    }
if (Ui_PointInRect(&rect_new_schedule, mx, my)) {
                Calendar all_calendars[32];
                int cal_count = CalMgr_GetAllCalendars(g_current_user.user_id, all_calendars, 32);

                // 1) 캘린더가 하나도 없으면, 먼저 캘린더 생성 화면부터 띄우기
                if (cal_count == 0) {
                    if (CalendarDialog_Add(g_current_user.user_id)) {
                        // 유저가 캘린더를 실제로 만들었다면 다시 목록 로드
                        cal_count = CalMgr_GetAllCalendars(g_current_user.user_id, all_calendars, 32);
                    }
                }

                // 2) 캘린더가 1개 이상 있으면 일정 등록 다이얼로그 띄우기
                if (cal_count > 0) {
                    Schedule new_schedule;
                    if (ScheduleDialog_Add(all_calendars, cal_count, &selected_date, &new_schedule)) {
                        new_schedule.is_deleted = 0;   // 삭제 플래그 초기화
                        FileIO_AddSchedule(&new_schedule);
                    }
                }

                need_redraw = 1;
            }
        }
        else if (ev.type == UI_INPUT_KEY) {
            if (ev.key == 27) {
                return SCENE_LOGIN;
            }
        }
    }
    
    return SCENE_EXIT;
}
