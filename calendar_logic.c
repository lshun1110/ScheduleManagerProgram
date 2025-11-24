#include "calendar_logic.h"
#include "file_io.h"
#include <stdio.h>
#include <string.h>

// calendar_logic.c는 이제 file_io.c로 통합되었으므로
// 여기서는 file_io 함수를 호출하도록 래퍼만 제공

int CalendarLogic_LoadAll(Calendar* buf, int max_count)
{
    return FileIO_LoadCalendars(buf, max_count);
}

int CalendarLogic_Add(const Calendar* cal)
{
    return FileIO_AddCalendar(cal);
}
