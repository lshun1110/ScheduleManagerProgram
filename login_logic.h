#ifndef LOGIN_LOGIC_H
#define LOGIN_LOGIC_H

#include "common.h"

int Login_Auth(const wchar_t* user_id, const wchar_t* password, User* out_user);

#endif