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
    // Basic validation: none of the fields should be empty.
    if (!id || !pw || !name || id[0] == L'\0' || pw[0] == L'\0' || name[0] == L'\0') {
        return 0;
    }

    // Load existing users to check for duplicates.
    User users[100];
    int count = FileIO_LoadUsers(users, 100);
    for (int i = 0; i < count; ++i) {
        if (users[i].is_deleted) {
            continue; // Skip deleted users
        }
        if (wcscmp(id, users[i].user_id) == 0) {
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

/*
 * Signup_CheckDuplicateId
 *
 * Returns 1 if the given ID is non-empty and not used by any
 * non-deleted user in users.txt, otherwise returns 0.
 */
int Signup_CheckDuplicateId(const wchar_t* id)
{
    if (id == NULL || id[0] == L'\0') {
        return 0; // empty ID is not allowed
    }

    User users[100];
    int count = FileIO_LoadUsers(users, 100);
    for (int i = 0; i < count; ++i) {
        if (users[i].is_deleted) {
            continue;
        }
        if (wcscmp(id, users[i].user_id) == 0) {
            return 0; // duplicate ID
        }
    }
    return 1; // OK to use
}