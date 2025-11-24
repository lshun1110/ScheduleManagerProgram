#ifndef CALENDAR_DIALOG_H
#define CALENDAR_DIALOG_H

#include "struct.h"

int CalendarDialog_Add(const wchar_t* user_id);
int CalendarDialog_Edit(Calendar* calendar);
int CalendarDialog_ConfirmDelete(const Calendar* calendar);

#endif
