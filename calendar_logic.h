#ifndef CALENDAR_LOGIC_H
#define CALENDAR_LOGIC_H

#include "struct.h"

/*
 * Loads all calendars from the persistent store (calendars.txt).  The
 * provided buffer must be large enough.  Returns the number of
 * calendars loaded.
 */
int CalendarLogic_LoadAll(Calendar* buf, int max_count);

/*
 * Adds a new calendar.  Returns 1 on success.
 */
int CalendarLogic_Add(const Calendar* cal);

#endif // CALENDAR_LOGIC_H