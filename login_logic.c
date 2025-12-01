#include "common.h"
#include "login_logic.h"
#include "file_io.h"

static FILE* OpenUserFileRead(void)
{
    return _wfopen(L"users.txt", L"rt, ccs=UTF-16LE");
}

static int ReadUserRecord(FILE* fp, User* out_user)
{
    wchar_t id[32];
    wchar_t pw[32];
    wchar_t name[32];
    int count;

    if (fp == NULL || out_user == NULL)
        return 0;

    count = fwscanf(fp, L"%31ls %31ls %31ls", id, pw, name);
    if (count != 3)
    {
        return 0;
    }

    wcsncpy(out_user->user_id, id, 31);
    out_user->user_id[31] = L'\0';

    wcsncpy(out_user->password, pw, 31);
    out_user->password[31] = L'\0';

    wcsncpy(out_user->name, name, 31);
    out_user->name[31] = L'\0';

    return 1;
}

int Login_Auth(const wchar_t* user_id,
    const wchar_t* password,
    User* out_user)
{
    User users[100];
    int count = FileIO_LoadUsers(users, 100);
    for (int i = 0; i < count; i++) {
        if (users[i].is_deleted) continue;
        if (wcscmp(user_id, users[i].user_id) == 0 &&
            wcscmp(password, users[i].password) == 0) {
            if (out_user) {
                *out_user = users[i];
            }
            return 1;
        }
    }
    return 0;
}