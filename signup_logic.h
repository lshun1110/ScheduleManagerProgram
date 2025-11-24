#ifndef SIGNUP_LOGIC_H
#define SIGNUP_LOGIC_H

// Ensure wchar_t is declared before using it in prototypes
#include <wchar.h>

/*
 * Attempts to create a new user.  The user ID must be unique in the
 * user.txt file.  Returns 1 on success and 0 on failure (e.g. empty
 * fields or duplicate ID).  The user.txt file is encoded in
 * UTF-16LE and stores one user per line in the format:
 *     ID PW NAME
 * separated by whitespace.  This function appends a new record to
 * that file when the ID does not already exist.
 */
int Signup_CreateUser(const wchar_t* id, const wchar_t* pw, const wchar_t* name);

/*
 * Checks whether the given user ID can be used for signup.
 * Returns 1 if the ID is non-empty and not used by any non-deleted
 * user in users.txt, otherwise returns 0.
 */
int Signup_CheckDuplicateId(const wchar_t* id);

#endif // SIGNUP_LOGIC_H