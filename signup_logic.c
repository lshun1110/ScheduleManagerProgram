#define _CRT_SECURE_NO_WARNINGS

#include "signup_logic.h"
#include "file_io.h"
#include <stdio.h>
#include <wchar.h>
#include <string.h>

/*
 * Signup_CreateUser
 *
 * Create a new user record.  If the ID already exists or any field
 * is empty, the function returns 0 and does nothing.  Otherwise it
 * adds the user via FileIO_AddUser with is_deleted set to 0.
 */
int Signup_CreateUser(const wchar_t* id, const wchar_t* pw, const wchar_t* name)
{
    if (!id || !pw || !name || id[0] == L'\0' || pw[0] == L'\0' || name[0] == L'\0') {
        return 0;
    }
    // Check for duplicate IDs using FileIO_LoadUsers
    User users[100];
    int count = FileIO_LoadUsers(users, 100);
    for (int i = 0; i < count; i++) {
        if (wcscmp(users[i].user_id, id) == 0 && !users[i].is_deleted) {
            return 0; // Duplicate
        }
    }
    // Create new user structure
    User new_user;
    wcsncpy(new_user.user_id, id, 31);
    new_user.user_id[31] = L'\0';
    wcsncpy(new_user.password, pw, 31);
    new_user.password[31] = L'\0';
    wcsncpy(new_user.name, name, 31);
    new_user.name[31] = L'\0';
    new_user.is_deleted = 0;
    // Add to users file
    return FileIO_AddUser(&new_user);
}