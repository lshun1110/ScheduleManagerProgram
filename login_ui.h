#ifndef LOGIN_UI_H
#define LOGIN_UI_H

#include "common.h"
#include "app_scene.h"

void draw_login_screen();
void print_title_segment();
void LoginUi_Draw(const wchar_t* id, const wchar_t* pw, LoginField currentField, int showPassword, int lastLoginFailed);
void LoginUi_UpdateFields(const wchar_t* id, const wchar_t* pw, LoginField currentField, int showPassword, int lastLoginFailed);

#endif
