#include "common.h"
#include "file_io.h"

static int next_calendar_id = 1;
static int next_schedule_id = 1;
static int next_share_id = 1;

// 사용자
int FileIO_LoadUsers(User* buf, int max_count)
{
    FILE* fp = _wfopen(L"users.txt", L"r, ccs=UTF-16LE");
    if (!fp) return 0;

    int count = 0;
    wchar_t line[256];

    while (count < max_count && fgetws(line, 256, fp)) {
        User u = { 0 };
        int scanned = swscanf_s(line,
            L"%31ls %31ls %31ls %d",
            u.user_id, 32,
            u.password, 32,
            u.name, 32,
            &u.is_deleted);

        if (scanned < 3) {
            continue; // 최소 3필드는 있어야 유효
        }
        if (scanned == 3) {
            u.is_deleted = 0; // is_deleted 생략되면 0으로
        }

        buf[count++] = u;
    }

    fclose(fp);
    return count;
}

int FileIO_AddUser(const User* user)
{
    FILE* fp = _wfopen(L"users.txt", L"a, ccs=UTF-16LE");
    if (!fp) return 0;

    // 필드 사이: 공백 한 칸 기준
    fwprintf(fp, L"%ls %ls %ls %d\n",
        user->user_id,
        user->password,
        user->name,
        user->is_deleted);

    fclose(fp);
    return 1;
}

int FileIO_UpdateUser(const User* user)
{
    User users[100];
    int count = FileIO_LoadUsers(users, 100);

    for (int i = 0; i < count; i++) {
        if (wcscmp(users[i].user_id, user->user_id) == 0) {
            users[i] = *user;
            break;
        }
    }

    FILE* fp = _wfopen(L"users.txt", L"w, ccs=UTF-16LE");
    if (!fp) return 0;

    for (int i = 0; i < count; i++) {
        fwprintf(fp, L"%ls %ls %ls %d\n",
            users[i].user_id,
            users[i].password,
            users[i].name,
            users[i].is_deleted);
    }

    fclose(fp);
    return 1;
}

int FileIO_DeleteUser(const wchar_t* user_id)
{
    User users[100];
    int count = FileIO_LoadUsers(users, 100);

    for (int i = 0; i < count; i++) {
        if (wcscmp(users[i].user_id, user_id) == 0) {
            users[i].is_deleted = 1;
            break;
        }
    }

    FILE* fp = _wfopen(L"users.txt", L"w, ccs=UTF-16LE");
    if (!fp) return 0;

    for (int i = 0; i < count; i++) {
        fwprintf(fp, L"%ls %ls %ls %d\n",
            users[i].user_id,
            users[i].password,
            users[i].name,
            users[i].is_deleted);
    }

    fclose(fp);
    return 1;
}

// 캘린더
int FileIO_LoadCalendars(Calendar* buf, int max_count) {
    FILE* fp = _wfopen(L"calendars.txt", L"r, ccs=UTF-16LE");
    if (!fp) return 0;
    
    int count = 0;
    wchar_t line[256];
    while (count < max_count && fgetws(line, 256, fp)) {
        Calendar c = {0};
        swscanf_s(line, L"%d\t%31[^\t]\t%31[^\t]\t%d\t%d\t%d",
                  &c.calendar_id, c.user_id, 32, c.name, 32,
                  &c.color, &c.is_active, &c.is_deleted);
        buf[count++] = c;
        if (c.calendar_id >= next_calendar_id) {
            next_calendar_id = c.calendar_id + 1;
        }
    }
    fclose(fp);
    return count;
}

int FileIO_AddCalendar(const Calendar* cal) {
    Calendar new_cal = *cal;
    if (new_cal.calendar_id == 0) {
        new_cal.calendar_id = next_calendar_id++;
    }
    
    FILE* fp = _wfopen(L"calendars.txt", L"a, ccs=UTF-16LE");
    if (!fp) return 0;
    fwprintf(fp, L"%d\t%ls\t%ls\t%d\t%d\t%d\n",
             new_cal.calendar_id, new_cal.user_id, new_cal.name,
             new_cal.color, new_cal.is_active, new_cal.is_deleted);
    fclose(fp);
    return new_cal.calendar_id;
}

int FileIO_UpdateCalendar(const Calendar* cal) {
    Calendar calendars[100];
    int count = FileIO_LoadCalendars(calendars, 100);
    
    for (int i = 0; i < count; i++) {
        if (calendars[i].calendar_id == cal->calendar_id) {
            calendars[i] = *cal;
            break;
        }
    }
    
    FILE* fp = _wfopen(L"calendars.txt", L"w, ccs=UTF-16LE");
    if (!fp) return 0;
    for (int i = 0; i < count; i++) {
        fwprintf(fp, L"%d\t%ls\t%ls\t%d\t%d\t%d\n",
                 calendars[i].calendar_id, calendars[i].user_id, calendars[i].name,
                 calendars[i].color, calendars[i].is_active, calendars[i].is_deleted);
    }
    fclose(fp);
    return 1;
}

int FileIO_DeleteCalendar(int calendar_id) {
    Calendar calendars[100];
    int count = FileIO_LoadCalendars(calendars, 100);
    if (count <= 0) return 0;

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (calendars[i].calendar_id == calendar_id) {
            calendars[i].is_deleted = 1;  // soft delete
            found = 1;
            break;
        }
    }
    if (!found) return 0;

    int total = count;
    int deleted = 0;
    for (int i = 0; i < count; i++) {
        if (calendars[i].is_deleted) {
            deleted++;
        }
    }

    FILE* fp = _wfopen(L"calendars.txt", L"w, ccs=UTF-16LE");
    if (!fp) return 0;

    int do_compact = 0;
    if (deleted >= CALENDAR_COMPACT_DELETE_COUNT ||
        (total > 0 && deleted * 2 > total)) {
        do_compact = 1;
    }

    for (int i = 0; i < count; i++) {
        if (do_compact && calendars[i].is_deleted) {
            // 압축 모드일 때는 삭제된 레코드는 파일에 안 씀
            continue;
        }
        fwprintf(fp, L"%d\t%ls\t%ls\t%d\t%d\t%d\n",
            calendars[i].calendar_id, calendars[i].user_id, calendars[i].name,
            calendars[i].color, calendars[i].is_active, calendars[i].is_deleted);
    }

    fclose(fp);
    return 1;
}

// 일정
int FileIO_LoadSchedules(Schedule* buf, int max_count) {
    FILE* fp = _wfopen(L"schedule.txt", L"r, ccs=UTF-16LE");
    if (!fp) return 0;
    
    int count = 0;
    wchar_t line[1024];
    while (count < max_count && fgetws(line, 1024, fp)) {
        Schedule s = {0};
        int y1, m1, d1, h1, min1, y2, m2, d2, h2, min2, repeat;
        
        swscanf_s(line, L"%d\t%d\t%63[^\t]\t%63[^\t]\t%255[^\t]\t%d-%d-%d %d:%d\t%d-%d-%d %d:%d\t%d\t%d\t%d",
                  &s.schedule_id, &s.calendar_id,
                  s.title, 64, s.location, 64, s.memo, 256,
                  &y1, &m1, &d1, &h1, &min1,
                  &y2, &m2, &d2, &h2, &min2,
                  &s.is_all_day, &repeat, &s.is_deleted);
        
        s.start_time.tm_year = y1 - 1900;
        s.start_time.tm_mon = m1 - 1;
        s.start_time.tm_mday = d1;
        s.start_time.tm_hour = h1;
        s.start_time.tm_min = min1;
        
        s.end_time.tm_year = y2 - 1900;
        s.end_time.tm_mon = m2 - 1;
        s.end_time.tm_mday = d2;
        s.end_time.tm_hour = h2;
        s.end_time.tm_min = min2;
        
        s.repeat_type = (RepeatType)repeat;
        // Trim memo placeholders used for empty memos
        if ((s.memo[0] == L' ' && s.memo[1] == 0) || wcscmp(s.memo, L"(empty)") == 0) {
            s.memo[0] = 0;
        }
        
        buf[count++] = s;
        if (s.schedule_id >= next_schedule_id) {
            next_schedule_id = s.schedule_id + 1;
        }
    }
    fclose(fp);
    return count;
}

int FileIO_AddSchedule(const Schedule* sched) {
    Schedule new_sched = *sched;
    if (new_sched.schedule_id == 0) {
        new_sched.schedule_id = next_schedule_id++;
    }
    
    FILE* fp = _wfopen(L"schedule.txt", L"a, ccs=UTF-16LE");
    if (!fp) return 0;
    
    fwprintf(fp, L"%d\t%d\t%ls\t%ls\t%ls\t%04d-%02d-%02d %02d:%02d\t%04d-%02d-%02d %02d:%02d\t%d\t%d\t%d\n",
             new_sched.schedule_id, new_sched.calendar_id,
             new_sched.title, new_sched.location, (new_sched.memo[0] ? new_sched.memo : L" " ),
             new_sched.start_time.tm_year + 1900, new_sched.start_time.tm_mon + 1, new_sched.start_time.tm_mday,
             new_sched.start_time.tm_hour, new_sched.start_time.tm_min,
             new_sched.end_time.tm_year + 1900, new_sched.end_time.tm_mon + 1, new_sched.end_time.tm_mday,
             new_sched.end_time.tm_hour, new_sched.end_time.tm_min,
             new_sched.is_all_day, new_sched.repeat_type, new_sched.is_deleted);
    fclose(fp);
    return new_sched.schedule_id;
}

int FileIO_UpdateSchedule(const Schedule* sched) {
    Schedule schedules[300];
    int count = FileIO_LoadSchedules(schedules, 1000);
    
    for (int i = 0; i < count; i++) {
        if (schedules[i].schedule_id == sched->schedule_id) {
            schedules[i] = *sched;
            break;
        }
    }
    
    FILE* fp = _wfopen(L"schedule.txt", L"w, ccs=UTF-16LE");
    if (!fp) return 0;
    for (int i = 0; i < count; i++) {
        fwprintf(fp, L"%d\t%d\t%ls\t%ls\t%ls\t%04d-%02d-%02d %02d:%02d\t%04d-%02d-%02d %02d:%02d\t%d\t%d\t%d\n",
                 schedules[i].schedule_id, schedules[i].calendar_id,
                 schedules[i].title, schedules[i].location, (schedules[i].memo[0] ? schedules[i].memo : L"(empty)"),
                 schedules[i].start_time.tm_year + 1900, schedules[i].start_time.tm_mon + 1, schedules[i].start_time.tm_mday,
                 schedules[i].start_time.tm_hour, schedules[i].start_time.tm_min,
                 schedules[i].end_time.tm_year + 1900, schedules[i].end_time.tm_mon + 1, schedules[i].end_time.tm_mday,
                 schedules[i].end_time.tm_hour, schedules[i].end_time.tm_min,
                 schedules[i].is_all_day, schedules[i].repeat_type, schedules[i].is_deleted);
    }
    fclose(fp);
    return 1;
}

int FileIO_DeleteSchedule(int schedule_id) {
    Schedule schedules[300];
    int count = FileIO_LoadSchedules(schedules, 1000);
    if (count <= 0) return 0;

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (schedules[i].schedule_id == schedule_id) {
            schedules[i].is_deleted = 1;  // soft delete
            found = 1;
            break;
        }
    }
    if (!found) return 0;

    int total = count;
    int deleted = 0;
    for (int i = 0; i < count; i++) {
        if (schedules[i].is_deleted) {
            deleted++;
        }
    }

    FILE* fp = _wfopen(L"schedule.txt", L"w, ccs=UTF-16LE");
    if (!fp) return 0;

    int do_compact = 0;
    if (deleted >= SCHEDULE_COMPACT_DELETE_COUNT ||
        (total > 0 && deleted * 2 > total)) {
        do_compact = 1;
    }

    for (int i = 0; i < count; i++) {
        if (do_compact && schedules[i].is_deleted) {
            // 압축 모드에서는 삭제된 일정은 파일에 안 씀
            continue;
        }
        fwprintf(fp, L"%d\t%d\t%ls\t%ls\t%ls\t"
            L"%04d-%02d-%02d %02d:%02d\t"
            L"%04d-%02d-%02d %02d:%02d\t"
            L"%d\t%d\t%d\n",
            schedules[i].schedule_id, schedules[i].calendar_id,
            schedules[i].title, schedules[i].location,
            (schedules[i].memo[0] ? schedules[i].memo : L"(empty)"),
            schedules[i].start_time.tm_year + 1900, schedules[i].start_time.tm_mon + 1, schedules[i].start_time.tm_mday,
            schedules[i].start_time.tm_hour, schedules[i].start_time.tm_min,
            schedules[i].end_time.tm_year + 1900, schedules[i].end_time.tm_mon + 1, schedules[i].end_time.tm_mday,
            schedules[i].end_time.tm_hour, schedules[i].end_time.tm_min,
            schedules[i].is_all_day, schedules[i].repeat_type, schedules[i].is_deleted);
    }

    fclose(fp);
    return 1;
}

// 공유
int FileIO_LoadShares(Share* buf, int max_count) {
    FILE* fp = _wfopen(L"shared_calendars.txt", L"r, ccs=UTF-16LE");
    if (!fp) return 0;
    
    int count = 0;
    wchar_t line[256];
    while (count < max_count && fgetws(line, 256, fp)) {
        Share s = {0};
        swscanf_s(line, L"%d\t%d\t%31[^\t]\t%31[^\t]\t%d\t%d",
                  &s.share_id, &s.calendar_id,
                  s.owner_id, 32, s.shared_with, 32,
                  &s.permission, &s.is_deleted);
        buf[count++] = s;
        if (s.share_id >= next_share_id) {
            next_share_id = s.share_id + 1;
        }
    }
    fclose(fp);
    return count;
}

int FileIO_AddShare(const Share* share) {
    Share new_share = *share;
    if (new_share.share_id == 0) {
        new_share.share_id = next_share_id++;
    }
    
    FILE* fp = _wfopen(L"shared_calendars.txt", L"a, ccs=UTF-16LE");
    if (!fp) return 0;
    fwprintf(fp, L"%d\t%d\t%ls\t%ls\t%d\t%d\n",
             new_share.share_id, new_share.calendar_id,
             new_share.owner_id, new_share.shared_with,
             new_share.permission, new_share.is_deleted);
    fclose(fp);
    return new_share.share_id;
}

int FileIO_DeleteShare(int share_id) {
    Share shares[100];
    int count = FileIO_LoadShares(shares, 100);
    
    for (int i = 0; i < count; i++) {
        if (shares[i].share_id == share_id) {
            shares[i].is_deleted = 1;
            break;
        }
    }
    
    FILE* fp = _wfopen(L"shared_calendars.txt", L"w, ccs=UTF-16LE");
    if (!fp) return 0;
    for (int i = 0; i < count; i++) {
        fwprintf(fp, L"%d\t%d\t%ls\t%ls\t%d\t%d\n",
                 shares[i].share_id, shares[i].calendar_id,
                 shares[i].owner_id, shares[i].shared_with,
                 shares[i].permission, shares[i].is_deleted);
    }
    fclose(fp);
    return 1;
}
