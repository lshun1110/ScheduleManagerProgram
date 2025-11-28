#define _CRT_SECURE_NO_WARNINGS

#include "signup_logic.h"
#include "file_io.h"
#include <stdio.h>
#include <wchar.h>
#include <string.h>


int Signup_CreateUser(const wchar_t* id, const wchar_t* pw, const wchar_t* name)
{
    if (!id || !pw || !name || id[0] == L'\0' || pw[0] == L'\0' || name[0] == L'\0') {
        return 0;
    }

    User users[100];
    int count = FileIO_LoadUsers(users, 100);
    for (int i = 0; i < count; ++i) {
        if (users[i].is_deleted) {
            continue; 
        }
        if (wcscmp(id, users[i].user_id) == 0) {
            return 0; 
        }
    }

    User new_user;
    wcsncpy(new_user.user_id, id, 31);
    new_user.user_id[31] = L'\0';
    wcsncpy(new_user.password, pw, 31);
    new_user.password[31] = L'\0';
    wcsncpy(new_user.name, name, 31);
    new_user.name[31] = L'\0';
    new_user.is_deleted = 0;

    return FileIO_AddUser(&new_user);
}

int Signup_CheckDuplicateId(const wchar_t* id)
{
    if (id == NULL || id[0] == L'\0') {
        return 0;
    }

    User users[100];
    int count = FileIO_LoadUsers(users, 100);
    for (int i = 0; i < count; ++i) {
        if (users[i].is_deleted) {
            continue;
        }
        if (wcscmp(id, users[i].user_id) == 0) {
            return 0;
        }
    }
    return 1; 
}